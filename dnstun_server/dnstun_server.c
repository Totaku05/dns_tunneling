#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include "mongoose.h"
#include "dnstun_defs.h"
#include "dnstun_resolver.h"
#include "dnstun_server.h"
#include "dnstun_cache.h"

typedef enum
{
    DNSTUN_SERVER_THREAD_STATE_FREE,
    DNSTUN_SERVER_THREAD_STATE_PROCESSING,
    DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED
} dnstun_server_thread_state_t;

typedef struct dnstun_server_data_s
{
    struct mg_connection *connection;
    char type[MAX_LENGTH_OF_TYPE];
    char name[MAX_LENGTH_OF_NAME];
} dnstun_server_data_t;

typedef struct dnstun_thread_ctx_s
{
    pthread_t thread;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    dnstun_server_data_t data;
    dnstun_server_thread_state_t thread_state;
} dnstun_thread_ctx_t;

struct dnstun_server_s
{
    struct mg_mgr manager;
    struct mg_connection *connection;
    ares_channel channel;
    dnstun_thread_ctx_t *contexts;
    int number_of_threads;
    dnstun_cache_t *cache;
};

static void *thread_handler(void *arg)
{
    int id;
    char answer[MAX_LENGTH_OF_ANSWER];
    char *result_of_request;
    int ttl;
    dnstun_server_t *dnstun_server = (dnstun_server_t*)arg;
    dnstun_thread_ctx_t *context;
    dnstun_server_data_t *data;

    for(id = 0; id < dnstun_server->number_of_threads; id++)
        if(dnstun_server->contexts[id].thread == pthread_self())
            break;

    context = &dnstun_server->contexts[id];
    data = &context->data;

    while(1)
    {
        pthread_mutex_lock(&context->mutex);

        while(context->thread_state == DNSTUN_SERVER_THREAD_STATE_FREE)
            pthread_cond_wait(&context->condition, &context->mutex);

        if(context->thread_state == DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED)
        {
            pthread_mutex_unlock(&context->mutex);
            break;
        }

        pthread_mutex_unlock(&context->mutex);

        if(dnstun_cache_find(dnstun_server->cache, data->type, data->name, answer) != DNSTUN_CACHE_RET_OK)
        {
            result_of_request = answer;

            sprintf(result_of_request, "%s, %s, ", data->type, data->name);
            result_of_request += strlen(result_of_request);

            dnstun_resolver_query(dnstun_server->channel, data->type, data->name, result_of_request, &ttl);

            dnstun_cache_insert(dnstun_server->cache, data->type, data->name, answer, ttl);
        }

        mg_send_head(data->connection, 200, strlen(answer), "Content-Type: text/plain");
        mg_printf(data->connection, "%.*s", (int)strlen(answer), answer);

        pthread_mutex_lock(&context->mutex);
        context->thread_state = DNSTUN_SERVER_THREAD_STATE_FREE;
        pthread_mutex_unlock(&context->mutex);
    }

    pthread_exit(0);
}

static void on_request(struct mg_connection *connection, int event_code, void *message, void *arg)
{
    if(event_code != MG_EV_HTTP_REQUEST)
        return;

    char type[MAX_LENGTH_OF_TYPE], name[MAX_LENGTH_OF_NAME];
    struct http_message *hm = (struct http_message*)message;
    dnstun_server_t *dnstun_server = (dnstun_server_t*)arg;
    int i;

    char *error_message = "The server is currently unavailable, please try again later.";

    mg_get_http_var(&(hm->query_string), "type", type, sizeof(type));
    mg_get_http_var(&(hm->query_string), "name", name, sizeof(name));

    syslog(LOG_INFO, "New request: %s %s", type, name);

    for(i = 0; i < dnstun_server->number_of_threads; i++)
    {
        pthread_mutex_lock(&(dnstun_server->contexts[i].mutex));

        if(dnstun_server->contexts[i].thread_state == DNSTUN_SERVER_THREAD_STATE_PROCESSING)
        {
            pthread_mutex_unlock(&(dnstun_server->contexts[i].mutex));
            continue;
        }

        if(dnstun_server->contexts[i].thread_state == DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED)
            goto exit_is_scheduled;

        dnstun_server->contexts[i].data.connection = connection;
        strcpy(dnstun_server->contexts[i].data.type, type);
        strcpy(dnstun_server->contexts[i].data.name, name);
        dnstun_server->contexts[i].thread_state = DNSTUN_SERVER_THREAD_STATE_PROCESSING;
        pthread_cond_signal(&(dnstun_server->contexts[i].condition));

    exit_is_scheduled:
        pthread_mutex_unlock(&(dnstun_server->contexts[i].mutex));
        return;
    }

    syslog(LOG_ERR, "There are no free threads to execute the query.");
    mg_send_head(connection, 503, strlen(error_message), "Content-Type: text/plain");
    mg_printf(connection, "%.*s", (int)strlen(error_message), error_message);
}

void dnstun_server_poll(dnstun_server_t *dnstun_server)
{
    mg_mgr_poll(&(dnstun_server->manager), 1000);
}

dnstun_server_ret_t dnstun_server_deinit(dnstun_server_t *dnstun_server)
{
    int i;
    for(i = 0; i < dnstun_server->number_of_threads; i++)
    {
        pthread_mutex_lock(&(dnstun_server->contexts[i].mutex));
        dnstun_server->contexts[i].thread_state = DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED;
        pthread_cond_signal(&(dnstun_server->contexts[i].condition));
        pthread_mutex_unlock(&(dnstun_server->contexts[i].mutex));

        pthread_join(dnstun_server->contexts[i].thread, NULL);
    }

    dnstun_resolver_deinit(dnstun_server->channel);
    dnstun_cache_deinit(dnstun_server->cache);
    mg_mgr_free(&(dnstun_server->manager));
    free(dnstun_server->contexts);
    free(dnstun_server);

    return DNSTUN_SERVER_RET_OK;
}

dnstun_server_ret_t dnstun_server_init(dnstun_server_t **dnstun_server, char *port, int num_of_threads, int num_of_items)
{
    dnstun_resolver_ret_t resolver_status;
    dnstun_cache_ret_t cache_status;
    int i, j;
    int thread_creation_status = 0;

    *dnstun_server = (dnstun_server_t*)malloc(sizeof(dnstun_server_t));
    if(*dnstun_server == NULL)
        return DNSTUN_SERVER_RET_FAIL;

    resolver_status = dnstun_resolver_init(&((*dnstun_server)->channel));
    if(resolver_status != DNSTUN_RESOLVER_RET_OK)
        goto resolver_initialization_error;

    cache_status = dnstun_cache_init(&((*dnstun_server)->cache), num_of_items);
    if(cache_status != DNSTUN_CACHE_RET_OK)
        goto cache_initialization_error;

    (*dnstun_server)->contexts = (dnstun_thread_ctx_t*)malloc(sizeof(dnstun_thread_ctx_t) * num_of_threads);
    if((*dnstun_server)->contexts == NULL)
        goto malloc_for_contexts_error;

    (*dnstun_server)->number_of_threads = num_of_threads;

    for(i = 0; i < num_of_threads; i++)
    {
        (*dnstun_server)->contexts[i].thread_state = DNSTUN_SERVER_THREAD_STATE_FREE;
        (*dnstun_server)->contexts[i].condition = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        (*dnstun_server)->contexts[i].mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        thread_creation_status = pthread_create(&((*dnstun_server)->contexts[i].thread), NULL, thread_handler, *dnstun_server);

        if(thread_creation_status)
        {
            syslog(LOG_ERR, "An error occurred while creating a thread");
            goto thread_creation_error;
        }
    }

    mg_mgr_init(&((*dnstun_server)->manager), NULL);
    (*dnstun_server)->connection = mg_bind(&((*dnstun_server)->manager), port, on_request, *dnstun_server);
    mg_set_protocol_http_websocket((*dnstun_server)->connection);

    return DNSTUN_SERVER_RET_OK;

thread_creation_error:
    for(j = 0; j < i; j++)
    {
        pthread_cancel((*dnstun_server)->contexts[j].thread);
        pthread_join((*dnstun_server)->contexts[j].thread, NULL);
    }
    free((*dnstun_server)->contexts);

malloc_for_contexts_error:
    dnstun_cache_deinit((*dnstun_server)->cache);

cache_initialization_error:
    dnstun_resolver_deinit((*dnstun_server)->channel);

resolver_initialization_error:
    free(*dnstun_server);
    return DNSTUN_SERVER_RET_FAIL;
}

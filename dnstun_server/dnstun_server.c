#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include "mongoose.h"
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
    dnstun_server_thread_state_t thread_state;
} dnstun_server_data_t;

struct dnstun_server_s
{
    struct mg_mgr manager;
    struct mg_connection *connection;
    ares_channel channel;
    dnstun_server_data_t *data;
    pthread_t *threads;
    pthread_cond_t *conditions;
    pthread_mutex_t *mutexes;
    int number_of_threads;
    dnstun_cache_t *cache;
};

static void *thread_handler(void *arg)
{
    int id;
    char *answer;
    char *result_of_request;
    int ttl;
    dnstun_server_t *dnstun_server = (dnstun_server_t*)arg;

    for(id = 0; id < dnstun_server->number_of_threads; id++)
        if(dnstun_server->threads[id] == pthread_self())
            break;

    pthread_mutex_lock(&(dnstun_server->mutexes[id]));
    while(1)
    {
        while(dnstun_server->data[id].thread_state == DNSTUN_SERVER_THREAD_STATE_FREE)
            pthread_cond_wait(&(dnstun_server->conditions[id]), &(dnstun_server->mutexes[id]));

        if(dnstun_server->data[id].thread_state == DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED)
            break;

        answer = (char*)malloc(MAX_LENGTH_OF_TYPE + MAX_LENGTH_OF_NAME + MAX_LENGTH_OF_HEADER + MAX_COUNT_OF_ANSWERS * MAX_LENGTH_OF_ANSWER);
        
        if(dnstun_cache_find(dnstun_server->cache, dnstun_server->data[id].type, dnstun_server->data[id].name, answer) != DNSTUN_CACHE_RET_OK)
        {
            result_of_request = answer;

            sprintf(result_of_request, "%s, %s, ", dnstun_server->data[id].type, dnstun_server->data[id].name);
            result_of_request += strlen(result_of_request);

            dnstun_resolver_query(dnstun_server->channel, dnstun_server->data[id].type, dnstun_server->data[id].name, result_of_request, &ttl);

            dnstun_cache_insert(dnstun_server->cache, dnstun_server->data[id].type, dnstun_server->data[id].name, answer, ttl);
        }

        mg_send_head(dnstun_server->data[id].connection, 200, strlen(answer), "Content-Type: text/plain");
        mg_printf(dnstun_server->data[id].connection, "%.*s", (int)strlen(answer), answer);

        dnstun_server->data[id].thread_state = DNSTUN_SERVER_THREAD_STATE_FREE;
        free(answer);
    }
    pthread_mutex_unlock(&(dnstun_server->mutexes[id]));

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

    mg_get_http_var(&(hm->query_string), "type", type, sizeof(type));
    mg_get_http_var(&(hm->query_string), "name", name, sizeof(name));

    syslog(LOG_INFO, "New request: %s %s", type, name);

    while(1)
    {
        for(i = 0; i < dnstun_server->number_of_threads; i++)
        {
            if(pthread_mutex_trylock(&(dnstun_server->mutexes[i])))
                continue;

            if(dnstun_server->data[i].thread_state == DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED)
                goto exit_is_scheduled;

            dnstun_server->data[i].connection = connection;
            strcpy(dnstun_server->data[i].type, type);
            strcpy(dnstun_server->data[i].name, name);
            dnstun_server->data[i].thread_state = DNSTUN_SERVER_THREAD_STATE_PROCESSING;
            pthread_cond_signal(&(dnstun_server->conditions[i]));

        exit_is_scheduled:
            pthread_mutex_unlock(&(dnstun_server->mutexes[i]));
            return;
        }
    }
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
        pthread_mutex_lock(&(dnstun_server->mutexes[i]));
        dnstun_server->data[i].thread_state = DNSTUN_SERVER_THREAD_STATE_EXIT_IS_SCHEDULED;
        pthread_cond_signal(&(dnstun_server->conditions[i]));
        pthread_mutex_unlock(&(dnstun_server->mutexes[i]));

        pthread_join(dnstun_server->threads[i], NULL);
    }

    dnstun_resolver_deinit(dnstun_server->channel);
    dnstun_cache_deinit(dnstun_server->cache);
    mg_mgr_free(&(dnstun_server->manager));
    free(dnstun_server->threads);
    free(dnstun_server->conditions);
    free(dnstun_server->mutexes);
    free(dnstun_server->data);
    free(dnstun_server);

    return DNSTUN_SERVER_RET_OK;
}

dnstun_server_ret_t dnstun_server_init(dnstun_server_t **dnstun_server, char *port, int num_of_threads, int num_of_items)
{
    dnstun_resolver_ret_t status;
    int i, j;
    int thread_creation_status = 0;

    *dnstun_server = (dnstun_server_t*)malloc(sizeof(dnstun_server_t));

    status = dnstun_resolver_init(&((*dnstun_server)->channel));
    if(status != DNSTUN_RESOLVER_RET_OK)
        goto resolver_initialization_error;

    (*dnstun_server)->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_of_threads);
    (*dnstun_server)->conditions = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * num_of_threads);
    (*dnstun_server)->mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_of_threads);
    (*dnstun_server)->data = (dnstun_server_data_t*)malloc(sizeof(dnstun_server_data_t) * num_of_threads);

    (*dnstun_server)->number_of_threads = num_of_threads;

    for(i = 0; i < num_of_threads; i++)
    {
        (*dnstun_server)->data[i].thread_state = DNSTUN_SERVER_THREAD_STATE_FREE;
        (*dnstun_server)->conditions[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        (*dnstun_server)->mutexes[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        thread_creation_status = pthread_create(&((*dnstun_server)->threads[i]), NULL, thread_handler, *dnstun_server);

        if(thread_creation_status)
        {
            syslog(LOG_ERR, "An error occurred while creating a thread");
            goto thread_creation_error;
        }
    }

    dnstun_cache_init(&((*dnstun_server)->cache), num_of_items);

    mg_mgr_init(&((*dnstun_server)->manager), NULL);
    (*dnstun_server)->connection = mg_bind(&((*dnstun_server)->manager), port, on_request, *dnstun_server);
    mg_set_protocol_http_websocket((*dnstun_server)->connection);

    return DNSTUN_SERVER_RET_OK;

thread_creation_error:
    for(j = 0; j < i; j++)
    {
        pthread_cancel((*dnstun_server)->threads[j]);
        pthread_join((*dnstun_server)->threads[j], NULL);
    }
    dnstun_resolver_deinit((*dnstun_server)->channel);
    free((*dnstun_server)->threads);
    free((*dnstun_server)->conditions);
    free((*dnstun_server)->mutexes);
    free((*dnstun_server)->data);
resolver_initialization_error:
    free(*dnstun_server);
    return DNSTUN_SERVER_RET_FAIL;
}

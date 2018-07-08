#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <ares.h>
#include <pthread.h>
#include "mongoose.h"
#include "dnstun_resolver.h"
#include "dnstun_server.h"

#define MAX_LENGTH_OF_TYPE 10
#define MAX_LENGTH_OF_NAME 100
#define MAX_LENGTH_OF_ANSWER 100
#define MAX_LENGTH_OF_HEADER 6

struct dnstun_server_s
{
    struct mg_mgr manager;
    struct mg_connection *connection;
    ares_channel channel;
};

struct dnstun_data_s
{
    struct mg_connection *connection;
    char type[MAX_LENGTH_OF_TYPE];
    char name[MAX_LENGTH_OF_NAME];
    int is_processed;
};

static struct dnstun_data_s *data;
static pthread_t *threads;
static pthread_cond_t *conditions;
static pthread_mutex_t *mutexes;

static void *thread_handler(void *arg)
{
    int id;
    char *answer;
    char *result_of_request;

    for(id = 0; data[id].connection != NULL; id++)
        if(threads[id] == pthread_self())
            break;

    pthread_mutex_lock(&mutexes[id]);
    while(1)
    {
        pthread_cond_wait(&conditions[id], &mutexes[id]);

        if(data[id].is_processed == -1)
            break;

        answer = (char*)malloc(MAX_LENGTH_OF_TYPE + MAX_LENGTH_OF_NAME + MAX_LENGTH_OF_HEADER + MAX_COUNT_OF_ANSWERS * MAX_LENGTH_OF_ANSWER);
        result_of_request = answer;

        sprintf(result_of_request, "%s, %s, ", data[id].type, data[id].name);
        result_of_request += strlen(result_of_request);

        dnstun_resolver_query(data[id].type, data[id].name, result_of_request);
    
        mg_send_head(data[id].connection, 200, strlen(answer), "Content-Type: text/plain");
        mg_printf(data[id].connection, "%.*s", (int)strlen(answer), answer);

        data[id].is_processed = 1;
        free(answer);
    }
    pthread_mutex_unlock(&mutexes[id]);

    pthread_exit(0);
}

static void on_request(struct mg_connection *connection, int event_code, void *message)
{
    if(event_code != MG_EV_HTTP_REQUEST)
        return;

    char type[MAX_LENGTH_OF_TYPE], name[MAX_LENGTH_OF_NAME];
    struct http_message *hm = (struct http_message*)message;
    int i;

    mg_get_http_var(&(hm->query_string), "type", type, sizeof(type));
    mg_get_http_var(&(hm->query_string), "name", name, sizeof(name));

    syslog(LOG_INFO, "New request: %s %s", type, name);

    while(1)
    {
        for(i = 0; data[i].is_processed != -1; i++)
        {
            if(!data[i].is_processed)
                continue;

            pthread_mutex_lock(&mutexes[i]);
            data[i].connection = connection;
            strcpy(data[i].type, type);
            strcpy(data[i].name, name);
            data[i].is_processed = 0;
            pthread_mutex_unlock(&mutexes[i]);

            pthread_cond_signal(&conditions[i]);
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
    for(i = 0; data[i].is_processed != -1; i++)
    {
        pthread_mutex_lock(&mutexes[i]);
        data[i].is_processed = -1;
        pthread_mutex_unlock(&mutexes[i]);

        pthread_cond_signal(&conditions[i]);
        pthread_join(threads[i], NULL);
    }

    dnstun_resolver_deinit(dnstun_server->channel);
    mg_mgr_free(&(dnstun_server->manager));
    free(dnstun_server);
    free(threads);
    free(conditions);
    free(mutexes);
    free(data);

    return DNSTUN_SERVER_RET_OK;
}

dnstun_server_ret_t dnstun_server_init(dnstun_server_t **dnstun_server, char *port, int num_of_threads)
{
    dnstun_resolver_ret_t status;
    int i, j;
    int thread_creation_status = 0;

    *dnstun_server = (dnstun_server_t*)malloc(sizeof(dnstun_server_t));

    status = dnstun_resolver_init(&((*dnstun_server)->channel));
    if(status != DNSTUN_RESOLVER_RET_OK)
    {
        free(*dnstun_server);
        return DNSTUN_SERVER_RET_FAIL;
    }

    threads = (pthread_t*)malloc(sizeof(pthread_t) * num_of_threads);
    conditions = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * num_of_threads);
    mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * num_of_threads);
    data = (struct dnstun_data_s*)malloc(sizeof(struct dnstun_data_s) * (num_of_threads + 1));

    for(i = 0; i < num_of_threads; i++)
    {
        data[i].is_processed = 1;
        conditions[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        mutexes[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        thread_creation_status = pthread_create(&threads[i], NULL, thread_handler, NULL);

        if(thread_creation_status)
        {
            syslog(LOG_ERR, "An error occurred while creating a thread");
            for(j = 0; j < i; j++)
            {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            dnstun_resolver_deinit((*dnstun_server)->channel);
            free(*dnstun_server);
            free(threads);
            free(conditions);
            free(mutexes);
            free(data);
            return DNSTUN_SERVER_RET_FAIL;
        }
    }
    data[num_of_threads].is_processed = -1;

    mg_mgr_init(&((*dnstun_server)->manager), NULL);
    (*dnstun_server)->connection = mg_bind(&((*dnstun_server)->manager), port, on_request);
    mg_set_protocol_http_websocket((*dnstun_server)->connection);

    return DNSTUN_SERVER_RET_OK;
}

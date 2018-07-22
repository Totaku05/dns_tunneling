#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "dnstun_defs.h"
#include "dnstun_cache.h"

typedef struct dnstun_cache_data_s
{
    char result_of_query[MAX_LENGTH_OF_ANSWER];
    char type[MAX_LENGTH_OF_TYPE];
    char name[MAX_LENGTH_OF_NAME];
    struct timespec timeout;
    struct dnstun_cache_data_s *next;
} dnstun_cache_data_t;

struct dnstun_cache_s
{
    dnstun_cache_data_t *head;
    dnstun_cache_data_t *tail;
    pthread_mutex_t mutex;
    int max_number_of_items;
    int current_number_of_items;
};

static void dnstun_cache_cleanup(dnstun_cache_t *dnstun_cache)
{
    struct timespec time;
    dnstun_cache_data_t *prev_item = NULL;
    dnstun_cache_data_t *current_item = dnstun_cache->head;
    dnstun_cache_data_t *tmp_item;

    while(current_item != NULL)
    {
        clock_gettime(CLOCK_REALTIME, &time);

        if(time.tv_sec < current_item->timeout.tv_sec || (time.tv_sec == current_item->timeout.tv_sec && time.tv_nsec < current_item->timeout.tv_nsec))
        {
            prev_item = current_item;
            current_item = current_item->next;
            continue;
        }

        if(prev_item == NULL)
            dnstun_cache->head = current_item->next;
        else
            prev_item->next = current_item->next;

        tmp_item = current_item;
        current_item = current_item->next;
        free(tmp_item);
        dnstun_cache->current_number_of_items--;
    }
    dnstun_cache->tail = prev_item;
}

dnstun_cache_ret_t dnstun_cache_find(dnstun_cache_t *dnstun_cache, char *type, char *name, char *result)
{
    struct timespec time;
    dnstun_cache_ret_t status = DNSTUN_CACHE_RET_FAIL;
    dnstun_cache_data_t *current_item = dnstun_cache->head;

    pthread_mutex_lock(&(dnstun_cache->mutex));
    while(current_item != NULL)
    {
        if(strcmp(current_item->type, type) || strcmp(current_item->name, name))
        {
            current_item = current_item->next;
            continue;
        }

        clock_gettime(CLOCK_REALTIME, &time);

        if(time.tv_sec < current_item->timeout.tv_sec || (time.tv_sec == current_item->timeout.tv_sec && time.tv_nsec < current_item->timeout.tv_nsec))
        {
            strcpy(result, current_item->result_of_query);
            status = DNSTUN_CACHE_RET_OK;
        }

        break;
    }
    pthread_mutex_unlock(&(dnstun_cache->mutex));
    return status;
}

dnstun_cache_ret_t dnstun_cache_insert(dnstun_cache_t *dnstun_cache, char *type, char *name, char *result, int ttl)
{
    dnstun_cache_ret_t status = DNSTUN_CACHE_RET_OK;
    dnstun_cache_data_t *tmp_item;
    struct timespec timeout;

    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += ttl;

    pthread_mutex_lock(&(dnstun_cache->mutex));

    dnstun_cache_cleanup(dnstun_cache);

    if(dnstun_cache->current_number_of_items == dnstun_cache->max_number_of_items)
    {
        tmp_item = dnstun_cache->head;
        dnstun_cache->head = dnstun_cache->head->next;
        free(tmp_item);
        dnstun_cache->current_number_of_items--;
    }

    tmp_item = (dnstun_cache_data_t*)malloc(sizeof(dnstun_cache_data_t));

    if(tmp_item == NULL)
    {
        status = DNSTUN_CACHE_RET_FAIL;
        goto malloc_error;
    }

    strcpy(tmp_item->type, type);
    strcpy(tmp_item->name, name);
    strcpy(tmp_item->result_of_query, result);
    tmp_item->timeout = timeout;
    tmp_item->next = NULL;

    if(!dnstun_cache->current_number_of_items)
        dnstun_cache->head = tmp_item;
    else
        dnstun_cache->tail->next = tmp_item;

    dnstun_cache->tail = tmp_item;
    dnstun_cache->current_number_of_items++;

malloc_error:
    pthread_mutex_unlock(&(dnstun_cache->mutex));
    return status;
}

dnstun_cache_ret_t dnstun_cache_init(dnstun_cache_t **dnstun_cache, int num_of_items)
{
    *dnstun_cache = (dnstun_cache_t*)malloc(sizeof(dnstun_cache_t));

    if(*dnstun_cache == NULL)
        return DNSTUN_CACHE_RET_FAIL;

    (*dnstun_cache)->head = NULL;
    (*dnstun_cache)->tail = NULL;
    (*dnstun_cache)->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    (*dnstun_cache)->max_number_of_items = num_of_items;
    (*dnstun_cache)->current_number_of_items = 0;

    return DNSTUN_CACHE_RET_OK;
}

dnstun_cache_ret_t dnstun_cache_deinit(dnstun_cache_t *dnstun_cache)
{
    dnstun_cache_data_t *current_item = dnstun_cache->head;
    dnstun_cache_data_t *tmp_item;

    while(current_item != NULL)
    {
        tmp_item = current_item;
        current_item = current_item->next;
        free(tmp_item);
    }
    free(dnstun_cache);

    return DNSTUN_CACHE_RET_OK;
}

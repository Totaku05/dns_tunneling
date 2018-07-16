#ifndef _DNSTUN_CACHE_H_
#define _DNSTUN_CACHE_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MAX_LENGTH_OF_TYPE 10
#define MAX_LENGTH_OF_NAME 100
#define MAX_LENGTH_OF_ANSWER 100
#define MAX_LENGTH_OF_HEADER 6

typedef enum
{
    DNSTUN_CACHE_RET_OK      = 0, /* Success */
    DNSTUN_CACHE_RET_FAIL    = 1, /* General fail */
} dnstun_cache_ret_t;

typedef struct dnstun_cache_s dnstun_cache_t;

/*****************************************************************************
 * dnstun_cache_init

 * @name                     dnstun_cache_init
 * @param   dnstun_cache     dns tunneling cache handle
 * @param   num_of_items     max number of items in a cache
 * @return  DNSTUN_CACHE_RET_OK if success
 *
 * Initializes cache
 */
dnstun_cache_ret_t dnstun_cache_init(dnstun_cache_t **dnstun_cache, int num_of_items);

/*****************************************************************************
 * dnstun_cache_deinit

 * @name                     dnstun_cache_deinit
 * @param   dnstun_cache     dns tunneling cache handle
 * @return  DNSTUN_CACHE_RET_OK if success
 *
 * Deinitializes cache
 */
dnstun_cache_ret_t dnstun_cache_deinit(dnstun_cache_t *dnstun_cache);

/*****************************************************************************
 * dnstun_cache_find

 * @name                     dnstun_cache_find
 * @param   dnstun_cache     dns tunneling cache handle
 * @param   type             type of a query
 * @param   name             name of a host
 * @param   result           points to a buffer in which a result should be put
 * @return  DNSTUN_CACHE_RET_OK if success
 *
 * Finds a result of a query with these type and host name in a cache. If there is no result of such a 
 * query in a cache, then this function returns DNSTUN_CACHE_RET_FAIL.
 */
dnstun_cache_ret_t dnstun_cache_find(dnstun_cache_t *dnstun_cache, char *type, char *name, char *result);

/*****************************************************************************
 * dnstun_cache_insert

 * @name                     dnstun_cache_insert
 * @param   dnstun_cache     dns tunneling cache handle
 * @param   type             type of a query
 * @param   name             name of a host
 * @param   result_of_query  result of a query
 * @param   ttl              ttl of a result of a query
 * @return  DNSTUN_CACHE_RET_OK if success
 *
 * Inserts a result of a query with a ttl in a cache.
 */
dnstun_cache_ret_t dnstun_cache_insert(dnstun_cache_t *dnstun_cache, char *type, char *name, char *result, int ttl);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_CACHE_H_ */

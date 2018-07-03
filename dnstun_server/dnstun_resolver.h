#ifndef _DNSTUN_RESOLVER_H_
#define _DNSTUN_RESOLVER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MAX_COUNT_OF_ANSWERS 100

typedef enum
{
    DNSTUN_RESOLVER_RET_OK              = 0, /* Success */
    DNSTUN_RESOLVER_RET_FAIL            = 1, /* General fail */
} dnstun_resolver_ret_t;

/*****************************************************************************
 * dnstun_resolver_init

 * @name                     dnstun_resolver_init
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Initializes c-ares libs and options
 */
dnstun_resolver_ret_t dnstun_resolver_init(void);

/*****************************************************************************
 * dnstun_resolver_deinit

 * @name                     dnstun_resolver_deinit
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Deinitializes c-ares libs and options
 */
dnstun_resolver_ret_t dnstun_resolver_deinit(void);

/*****************************************************************************
 * dnstun_resolver_query

 * @name                     dnstun_resolver_query
 * @param   type             type of a query
 * @param   name             name of a host
 * @param   answer           points to a buffer in which a result should be put
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Initiate a single-question DNS query
 */
dnstun_resolver_ret_t dnstun_resolver_query(char *type, char *name, char *answer);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_RESOLVER_H_ */

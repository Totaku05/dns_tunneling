#ifndef _DNSTUN_RESOLVER_H_
#define _DNSTUN_RESOLVER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <ares.h>

#define MAX_COUNT_OF_ANSWERS 100

typedef enum
{
    DNSTUN_RESOLVER_RET_OK   = 0, /* Success */
    DNSTUN_RESOLVER_RET_FAIL = 1, /* General fail */
} dnstun_resolver_ret_t;

/*****************************************************************************
 * dnstun_resolver_init

 * @name                     dnstun_resolver_init
 * @param   channel          a pointer to a name service channel
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Initializes resolver
 */
dnstun_resolver_ret_t dnstun_resolver_init(ares_channel *channel);

/*****************************************************************************
 * dnstun_resolver_deinit

 * @name                     dnstun_resolver_deinit
 * @param   channel          a name service channel
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Deinitializes resolver
 */
dnstun_resolver_ret_t dnstun_resolver_deinit(ares_channel channel);

/*****************************************************************************
 * dnstun_resolver_query

 * @name                     dnstun_resolver_query
 * @param   channel          a name service channel
 * @param   type             type of a query
 * @param   name             name of a host
 * @param   answer           points to a buffer in which a result should be put
 * @param   ttl              points to a variable in which a ttl should be put
 * @return  DNSTUN_RESOLVER_RET_OK if success
 *
 * Initiate a single-question DNS query
 */
dnstun_resolver_ret_t dnstun_resolver_query(ares_channel channel, char *type, char *name, char *answer, int *ttl);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_RESOLVER_H_ */

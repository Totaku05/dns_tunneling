#ifndef _DNSTUN_SERVER_H_
#define _DNSTUN_SERVER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DNSTUN_SERVER_VERSION 3.0

typedef enum
{
    DNSTUN_SERVER_RET_OK     = 0, /* Success */
    DNSTUN_SERVER_RET_FAIL   = 1, /* General fail */
} dnstun_server_ret_t;

typedef struct dnstun_server_s dnstun_server_t;

/*****************************************************************************
 * dnstun_server_init

 * @name                     dnstun_server_init
 * @param   dnstun_server    dns tunneling server handle
 * @param   port             port on which a server will be raised
 * @param   num_of_threads   number of threads that a server will create
 * @param   num_of_items     max number of items in a cache
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Initializes server
 */
dnstun_server_ret_t dnstun_server_init(dnstun_server_t **dnstun_server, char *port, int num_of_threads, int num_of_items);

/*****************************************************************************
 * dnstun_server_deinit

 * @name                     dnstun_server_deinit
 * @param   dnstun_server    dns tunneling server handle
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Deinitializes server
 */
dnstun_server_ret_t dnstun_server_deinit(dnstun_server_t *dnstun_server);

/*****************************************************************************
 * dnstun_server_poll

 * @name                     dnstun_server_poll
 * @param   dnstun_server    dns tunneling server handle
 *
 * Iterates over all sockets, accepts new connections, sends and receives data, closes connections and 
 * calls event handler functions for the respective events
 */
void dnstun_server_poll(dnstun_server_t *dnstun_server);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_SERVER_H_ */

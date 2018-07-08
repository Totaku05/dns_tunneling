#ifndef _DNSTUN_SERVER_H_
#define _DNSTUN_SERVER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DNSTUN_SERVER_VERSION 2.0

typedef enum
{
    DNSTUN_SERVER_RET_OK              = 0, /* Success */
    DNSTUN_SERVER_RET_FAIL            = 1, /* General fail */
} dnstun_server_ret_t;

typedef struct dnstun_server_s dnstun_server_t;

/*****************************************************************************
 * dnstun_server_init

 * @name                     dnstun_server_init
 * @param   dnstun_server    a double pointer to a struct in which a server will put
 *                           mongoose manager and connection and c-ares channel
 * @param   port             a port on which a server will be raised
 * @param   num_of_threads   a number of threads that a server will create
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Initializes dnstun_resolver and mongoose manager and connection
 */
dnstun_server_ret_t dnstun_server_init(dnstun_server_t **dnstun_server, char *port, int num_of_threads);

/*****************************************************************************
 * dnstun_server_deinit

 * @name                     dnstun_server_deinit
 * @param   dnstun_server    a pointer to a struct in which contains mongoose manager and connection
 *                           and c-ares channel
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Deinitializes dnstun_resolver, mongoose manager and connection, stops threads
 */
dnstun_server_ret_t dnstun_server_deinit(dnstun_server_t *dnstun_server);

/*****************************************************************************
 * dnstun_server_poll

 * @name                     dnstun_server_poll
 * @param   dnstun_server    a pointer to a struct in which contains mongoose manager and connection
 *                           and c-ares channel
 *
 * Iterates over all sockets, accepts new connections, sends and receives data, closes connections and 
 * calls event handler functions for the respective eventsDeinitializes dnstun_resolver and mongoose 
 * manager and connection
 */
void dnstun_server_poll(dnstun_server_t *dnstun_server);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_SERVER_H_ */

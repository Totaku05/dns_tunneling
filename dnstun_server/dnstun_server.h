#ifndef _DNSTUN_SERVER_H_
#define _DNSTUN_SERVER_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DNSTUN_SERVER_VERSION 1.0

typedef enum
{
    DNSTUN_SERVER_RET_OK              = 0, /* Success */
    DNSTUN_SERVER_RET_FAIL            = 1, /* General fail */
} dnstun_server_ret_t;

/*****************************************************************************
 * dnstun_server_init

 * @name                     dnstun_server_init
 * @param   port             a port on which a server will be raised
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Initializes dnstun_resolver and mongoose manager and connection
 */
dnstun_server_ret_t dnstun_server_init(char *port);

/*****************************************************************************
 * dnstun_server_deinit

 * @name                     dnstun_server_deinit
 * @return  DNSTUN_SERVER_RET_OK if success
 *
 * Deinitializes dnstun_resolver and mongoose manager and connection
 */
dnstun_server_ret_t dnstun_server_deinit(void);

/*****************************************************************************
 * dnstun_server_poll

 * @name                     dnstun_server_poll
 *
 * Iterates over all sockets, accepts new connections, sends and receives data, closes connections and 
 * calls event handler functions for the respective eventsDeinitializes dnstun_resolver and mongoose 
 * manager and connection
 */
void dnstun_server_poll(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_SERVER_H_ */

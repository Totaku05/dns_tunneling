#ifndef _DNSTUN_CLIENT_H_
#define _DNSTUN_CLIENT_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DNSTUN_CLIENT_VERSION 4.0

#define CLIENT_EXPORT __attribute__ ((visibility("default")))
#define CLIENT_LOCAL __attribute__ ((visibility("hidden")))

typedef enum
{
    DNSTUN_CLIENT_RET_OK   = 0, /* Success */
    DNSTUN_CLIENT_RET_FAIL = 1, /* General fail */
} dnstun_client_ret_t;

typedef struct dnstun_client_s dnstun_client_t;

/*****************************************************************************
 * dnstun_client_init

 * @name                     dnstun_client_init
 * @param   dnstun_client    dns tunneling client handle
 * @param   stream           stream to which the result of the query is output
 * @return  DNSTUN_CLIENT_RET_OK if success
 *
 * Initializes client
 */
CLIENT_EXPORT dnstun_client_ret_t dnstun_client_init(dnstun_client_t **dnstun_client, FILE *stream);

/*****************************************************************************
 * dnstun_client_deinit

 * @name                     dnstun_client_deinit
 * @param   dnstun_client    dns tunneling client handle
 * @return  DNSTUN_CLIENT_RET_OK if success
 *
 * Deinitializes client
 */
CLIENT_EXPORT dnstun_client_ret_t dnstun_client_deinit(dnstun_client_t *dnstun_client);

/*****************************************************************************
 * dnstun_client_send_request

 * @name                     dnstun_client_send_request
 * @param   dnstun_client    dns tunneling client handle
 * @param   url              string with the dnstun_server address, which includes such parameters of the
 *                           GET request as the request type and the host name
 * @return  DNSTUN_CLIENT_RET_OK if success
 *
 * Sends a GET request. We assume that url has follow format: 
 * http://somehost:someport/?type=sometype&name=somename
 */
CLIENT_EXPORT dnstun_client_ret_t dnstun_client_send_request(dnstun_client_t *dnstun_client, char *url);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_CLIENT_H_ */

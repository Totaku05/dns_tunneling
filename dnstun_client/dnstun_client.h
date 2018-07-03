#ifndef _DNSTUN_CLIENT_H_
#define _DNSTUN_CLIENT_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define DNSTUN_CLIENT_VERSION 1.0

typedef enum
{
    DNSTUN_CLIENT_RET_OK   = 0, /* Success */
    DNSTUN_CLIENT_RET_FAIL = 1, /* General fail */
} dnstun_client_ret_t;

/*****************************************************************************
 * dnstun_client_send_request

 * @name                     dnstun_client_send_request
 * @param   url              string with the dnstun_server address, which includes such parameters of the
 *                           GET request as the request type and the host name
 * @param   stream           stream to which the result of the query is output
 * @return  DNSTUN_CLIENT_RET_OK if success
 *
 * Sends a GET reques. We assume that url has follow format: 
 * http://somehost:someport/?type=sometype&name=somename
 */
dnstun_client_ret_t dnstun_client_send_request(char *url, FILE *stream);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _DNSTUN_CLIENT_H_ */

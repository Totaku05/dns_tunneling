#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"
#include "dnstun_resolver.h"
#include "dnstun_server.h"

#define MAX_LENGTH_OF_TYPE 10
#define MAX_LENGTH_OF_NAME 100
#define MAX_LENGTH_OF_ANSWER 100
#define MAX_LENGTH_OF_HEADER 6

struct mg_mgr manager;
struct mg_connection *connection;

static void on_request(struct mg_connection *con, int event_code, void *message)
{
    if(event_code != MG_EV_HTTP_REQUEST)
        return;

    char type[MAX_LENGTH_OF_TYPE], name[MAX_LENGTH_OF_NAME];
    int code = -1;
    struct http_message *hm = (struct http_message*)message;
    char *answer = (char*)malloc(MAX_LENGTH_OF_TYPE + MAX_LENGTH_OF_NAME + MAX_LENGTH_OF_HEADER + MAX_COUNT_OF_ANSWERS * MAX_LENGTH_OF_ANSWER);
    char *result_of_request = answer;

    mg_get_http_var(&(hm->query_string), "type", type, sizeof(type));
    mg_get_http_var(&(hm->query_string), "name", name, sizeof(name));

    printf("New request: %s %s\n", type, name);

    sprintf(result_of_request, "%s, %s, ", type, name);
    result_of_request += strlen(result_of_request);

    dnstun_resolver_query(type, name, result_of_request);
    
    mg_send_head(con, 200, strlen(answer), "Content-Type: text/plain");
    mg_printf(con, "%.*s", (int)strlen(answer), answer);

    free(answer);
}

void dnstun_server_poll(void)
{
    mg_mgr_poll(&manager, 1000);
}

dnstun_server_ret_t dnstun_server_deinit(void)
{
    dnstun_resolver_deinit();
    mg_mgr_free(&manager);

    return DNSTUN_SERVER_RET_OK;
}

dnstun_server_ret_t dnstun_server_init(char *port)
{
    dnstun_resolver_ret_t status;

    status = dnstun_resolver_init();
    if(status != DNSTUN_RESOLVER_RET_OK)
        return DNSTUN_SERVER_RET_FAIL;

    mg_mgr_init(&manager, NULL);
    connection = mg_bind(&manager, port, on_request);
    mg_set_protocol_http_websocket(connection);

    return DNSTUN_SERVER_RET_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "dnstun_client.h"

struct dnstun_client_s
{
    CURL *curl;
};

static size_t on_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    char *data = (char*)ptr;
    char response[220];
    char *result_of_request;
    size_t written = 0;
    int length = 0;
    char *current_line = NULL;
    char *next_line = strchr(data, '\n');

    if(!next_line)
    {
        sprintf(response, "%s\n", data);
        written = fwrite(response, strlen(response), 1, (FILE *)stream);
        return written;
    }

    length = next_line - data;

    strncpy(response, data, length);
    result_of_request = response + length;
    strcpy(result_of_request, ", ");
    result_of_request += 2;

    while(current_line = next_line ? next_line + 1 : NULL)
    {
        next_line = strchr(current_line, '\n');
        if(!next_line) 
            break;
        length = next_line - current_line + 1;
        strncpy(result_of_request, current_line, length);
        result_of_request[length] = 0;
        written += fwrite(response, strlen(response), 1, (FILE *)stream);
    }
    return written;
}

dnstun_client_ret_t dnstun_client_send_request(dnstun_client_t *dnstun_client, char *url)
{
    curl_easy_setopt(dnstun_client->curl, CURLOPT_URL, url);
    curl_easy_perform(dnstun_client->curl);

    return DNSTUN_CLIENT_RET_OK;
}

dnstun_client_ret_t dnstun_client_init(dnstun_client_t **dnstun_client, FILE *stream)
{
    *dnstun_client = (dnstun_client_t*)malloc(sizeof(dnstun_client_t));

    (*dnstun_client)->curl = curl_easy_init();

    if(!(*dnstun_client)->curl)
        return DNSTUN_CLIENT_RET_FAIL;

    curl_easy_setopt((*dnstun_client)->curl, CURLOPT_WRITEFUNCTION, on_response);
    curl_easy_setopt((*dnstun_client)->curl, CURLOPT_WRITEDATA, stream);

    return DNSTUN_CLIENT_RET_OK;
}

dnstun_client_ret_t dnstun_client_deinit(dnstun_client_t *dnstun_client)
{
    curl_easy_cleanup(dnstun_client->curl);
    free(dnstun_client);

    return DNSTUN_CLIENT_RET_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "dnstun_client.h"

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

dnstun_client_ret_t dnstun_client_send_request(char *url, FILE *stream)
{
    CURL *curl = curl_easy_init();

    if(!curl)
        return DNSTUN_CLIENT_RET_FAIL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return DNSTUN_CLIENT_RET_OK;
}

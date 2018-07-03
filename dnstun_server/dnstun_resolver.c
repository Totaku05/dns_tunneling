#include <ares.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include "dnstun_resolver.h"

static ares_channel channel;

typedef struct data_for_dns_request_s
{
    char *answer;
    char *type;
    char *name;
} data_for_dns_request_t;

static int get_code_by_type(char *type)
{
    int code = -1;

    if(!strcmp(type, "A")) 
        code = ns_t_a;
    else if(!strcmp(type, "TXT"))
        code = ns_t_txt;
    else if(!strcmp(type, "MX"))
        code = ns_t_mx;

    return code;
}

static void parse_a(unsigned char *abuf, int alen, char* answer)
{
    int i = 0;
    struct ares_addrttl info[MAX_COUNT_OF_ANSWERS];
    int count = MAX_COUNT_OF_ANSWERS;
    int status = 0;
    char *address;

    status = ares_parse_a_reply(abuf, alen, NULL, info, &count);

    if(status != ARES_SUCCESS){
        printf("Failed to lookup: %s\n", ares_strerror(status));
        strcpy(answer, "1\0");
        return;
    }

    strcpy(answer, "0\n");
    answer += 2;

    for(; i < count; i++)
    {
        address = inet_ntoa(info[i].ipaddr);
        printf("%s\n", address);
        sprintf(answer, "%s\n", address);
        answer += strlen(address) + 1;
    }
    answer = 0;
}

static void parse_txt(unsigned char *abuf, int alen, char *answer)
{
    struct ares_txt_reply *txt_out = NULL, *tmp_txt;
    int status = 0;

    status = ares_parse_txt_reply(abuf, alen, &txt_out);

    if(!txt_out || status != ARES_SUCCESS)
    {
        printf("Failed to lookup: %s\n", ares_strerror(status));
        strcpy(answer, "1\0");
        return;
    }

    tmp_txt = txt_out;

    strcpy(answer, "0\n");
    answer += 2;

    for(; tmp_txt; tmp_txt = tmp_txt->next)
    {
        printf("%s\n", tmp_txt->txt);
        sprintf(answer, "%s\n", tmp_txt->txt);
        answer += strlen(tmp_txt->txt) + 1;
    }
    answer = 0;

    ares_free_data(txt_out);
}

static void parse_mx(unsigned char *abuf, int alen, char *answer)
{
    struct ares_mx_reply *mx_out = NULL, *tmp_mx;
    int status = 0;

    status = ares_parse_mx_reply(abuf, alen, &mx_out);

    if(!mx_out || status != ARES_SUCCESS)
    {
        printf("Failed to lookup: %s\n", ares_strerror(status));
        strcpy(answer, "1\0");
        return;
    }

    tmp_mx = mx_out;

    strcpy(answer, "0\n");
    answer += 2;

    for(; tmp_mx; tmp_mx = tmp_mx->next)
    {
        printf("%d %s\n", tmp_mx->priority, tmp_mx->host);
        sprintf(answer, "%d %s\n", tmp_mx->priority, tmp_mx->host);
        answer += strlen(answer);
    }
    answer = 0;

    ares_free_data(mx_out);
}

static void on_dns_response(void *arg, int status, int timeouts, unsigned char *abuf, int alen)
{
    data_for_dns_request_t *data = (data_for_dns_request_t*)arg;
    int code = get_code_by_type(data->type);

    if(!abuf || status != ARES_SUCCESS){
        printf("The %s %s request wasn't successful.\n", data->type, data->name);
        strcpy(data->answer, "1\0");
        return;
    }

    switch(code)
    {
        case ns_t_a: 
            parse_a(abuf, alen, data->answer);
            break;
        case ns_t_txt:
            parse_txt(abuf, alen, data->answer);
            break;
        case ns_t_mx:
            parse_mx(abuf, alen, data->answer);
            break;
    }
}

static void wait_for_response(void)
{
    while(1)
    {
        struct timeval *tvp, tv;
        fd_set read_fds, write_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);

        if(!nfds)
            break;

        tvp = ares_timeout(channel, NULL, &tv);
        select(nfds, &read_fds, &write_fds, NULL, tvp);
        ares_process(channel, &read_fds, &write_fds);
    }
}

dnstun_resolver_ret_t dnstun_resolver_query(char *type, char *name, char *answer)
{
    data_for_dns_request_t data;
    int code = get_code_by_type(type);

    if(code != -1)
    {
        data.answer = answer;
        data.type = type;
        data.name = name;

        ares_query(channel, name, ns_c_in, code, on_dns_response, &data);
        wait_for_response();
    }
    else
    {
        printf("This request has the wrong type.\n");
        strcpy(answer, "1\0");
    }

    return DNSTUN_RESOLVER_RET_OK;
}

dnstun_resolver_ret_t dnstun_resolver_deinit(void)
{
    ares_destroy(channel);
    ares_library_cleanup();

    return DNSTUN_RESOLVER_RET_OK;
}

dnstun_resolver_ret_t dnstun_resolver_init(void)
{
    int status;
    struct ares_options options;
    int optmask = 0;

    status = ares_library_init(ARES_LIB_INIT_ALL);
    if(status != ARES_SUCCESS)
    {
        printf("ares_library_init: %s\n", ares_strerror(status));
        return DNSTUN_RESOLVER_RET_FAIL;
    }

    status = ares_init_options(&channel, &options, optmask);
    if(status != ARES_SUCCESS)
    {
        printf("ares_init_options: %s\n", ares_strerror(status));
        return DNSTUN_RESOLVER_RET_FAIL;
    }

    return DNSTUN_RESOLVER_RET_OK;
}

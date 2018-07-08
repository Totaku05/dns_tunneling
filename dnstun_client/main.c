#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "dnstun_client.h"

static void s_print_usage(char *name_of_executable_file)
{
    fprintf(stderr, "DNS tunneling client version: %.1f\n", DNSTUN_CLIENT_VERSION);
    fprintf(stderr, "Usage: %s\n", name_of_executable_file);
    fprintf(stderr, "[-h/--help - Show the possible options]\n");
    fprintf(stderr, "[-a/--address <address> - Enter the server address (the default address is "
                    "http://localhost:8000/)]\n");
    fprintf(stderr, "[-s/--source_file <file name> - Enter the source file name (the default name is"
                    " test)]\n");
    fprintf(stderr, "[-d/--destination_file <file name> - Enter the destination file name (the default"
                    " output is to the console)]\n");
}

int main(int argc, char *argv[])
{
    char *src_name = "test";
    char *dst_name = NULL;
    char *server = "http://localhost:8000/";
    FILE *src = NULL;
    FILE *dst = stdout;

    char line[120];
    char url[220];
    char type[10];
    char name[100];

    dnstun_client_ret_t status;
    dnstun_client_t *dnstun_client;

    int opt;
    struct option options[5];

    options[0].name = "help";
    options[0].has_arg = no_argument;
    options[0].val = 'h';
    options[0].flag = NULL;

    options[1].name = "address";
    options[1].has_arg = required_argument;
    options[1].val = 'a';
    options[1].flag = NULL;

    options[2].name = "source_file";
    options[2].has_arg = required_argument;
    options[2].val = 's';
    options[2].flag = NULL;

    options[3].name = "destination_file";
    options[3].has_arg = required_argument;
    options[3].val = 'd';
    options[3].flag = NULL;

    options[4].name = NULL;

    while ((opt = getopt_long(argc, argv, "ha:s:d:", options, NULL)) != -1)
    {
        switch(opt)
        {
            case 'h':
                s_print_usage(argv[0]);
                return 0;
            case 'a':
                server = optarg;
                break;
            case 's':
                src_name = optarg;
                break;
            case 'd':
                dst_name = optarg;
                break;
            default:
                return -1;
        }
    }

    src = fopen(src_name, "r");
    if(dst_name)
        dst = fopen(dst_name, "w");

    status = dnstun_client_init(&dnstun_client, dst);
    if(status != DNSTUN_CLIENT_RET_OK)
    {
        fclose(src);
        if(dst_name)
            fclose(dst);
        return status;
    }

    while (fgets(line, sizeof(line), src)) {
        sscanf(line, "%[A-Z], %s", type, name);
        sprintf(url, "%s?type=%s&name=%s", server, type, name);

        dnstun_client_send_request(dnstun_client, url);
    }

    dnstun_client_deinit(dnstun_client);
    fclose(src);
    if(dst_name)
        fclose(dst);
    return status;
}

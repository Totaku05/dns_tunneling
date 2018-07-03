#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnstun_client.h"

static void s_print_usage(char *name_of_executable_file)
{
    fprintf(stderr, "DNS tunneling client version: %.1f\n", DNSTUN_CLIENT_VERSION);
    fprintf(stderr, "Usage: %s\n", name_of_executable_file);
    fprintf(stderr, "[-h/--help - Show the possible options]\n");
    fprintf(stderr, "[-s/--server <address> - Enter the server address (the default address is "
                    "http://localhost:8000/)]\n");
    fprintf(stderr, "[-src/--source_file <file name> - Enter the source file name (the default name is"
                    " test)]\n");
    fprintf(stderr, "[-dst/--destination_file <file name> - Enter the destination file name (the default"
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

    int i = 1;
    char *arg;

    for(; i < argc; i++)
    {
        arg = argv[i];

        if(!strcmp(arg, "-h") || !strcmp(arg, "--help"))
        {
            s_print_usage(argv[0]);
            return 0;
        }
        if(!strcmp(arg, "-s") || !strcmp(arg, "--server"))
        {
            if(++i >= argc)
                break;
            else
                server = argv[i];
        }
        else if(!strcmp(arg, "-src") || !strcmp(arg, "--source_file"))
        {
            if(++i >= argc)
                break;
            else
                src_name = argv[i];
        }
        else if(!strcmp(arg, "-dst") || !strcmp(arg, "--destination_file"))
        {
            if(++i >= argc)
                break;
            else
                dst_name = argv[i];
        }
    }

    src = fopen(src_name, "r");
    if(dst_name)
        dst = fopen(dst_name, "w");

    while (fgets(line, sizeof(line), src)) {
        sscanf(line, "%[A-Z], %s", type, name);
        sprintf(url, "%s?type=%s&name=%s", server, type, name);

        status = dnstun_client_send_request(url, dst);
        if(status != DNSTUN_CLIENT_RET_OK)
            break;
    }

    fclose(src);
    if(dst_name)
        fclose(dst);
    return status;
}

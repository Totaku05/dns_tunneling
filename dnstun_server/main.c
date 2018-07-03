#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnstun_server.h"

static void s_print_usage(char *name_of_executable_file)
{
    fprintf(stderr, "DNS tunneling server version: %.1f\n", DNSTUN_SERVER_VERSION);
    fprintf(stderr, "Usage: %s\n", name_of_executable_file);
    fprintf(stderr, "[-h/--help - Show the possible options]\n");
    fprintf(stderr, "[-p/--port <port> - Enter the port (the default port is 8000)]\n");
}

int main(int argc, char *argv[])
{
    dnstun_server_ret_t status;

    char *port = "8000";

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
        if(!strcmp(arg, "-p") || !strcmp(arg, "--port"))
        {
            if(++i >= argc)
                break;
            else
                port = argv[i];
        }
    }

    status = dnstun_server_init(port);
    if(status != DNSTUN_SERVER_RET_OK)
        return -1;

    while(1)
        dnstun_server_poll();

    dnstun_server_deinit();
    return 0;
}

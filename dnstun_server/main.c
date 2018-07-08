#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <syslog.h>
#include <signal.h>
#include "dnstun_server.h"

static int stop = 0;

static void s_stop_server(int sig)
{
    stop = 1;
}

static void s_print_usage(char *name_of_executable_file)
{
    fprintf(stderr, "DNS tunneling server version: %.1f\n", DNSTUN_SERVER_VERSION);
    fprintf(stderr, "Usage: %s\n", name_of_executable_file);
    fprintf(stderr, "[-h/--help - Show the possible options]\n");
    fprintf(stderr, "[-p/--port <port> - Enter the port (the default port is 8000)]\n");
    fprintf(stderr, "[-l/--level <level> - Enter the level of logging (the default level is info). "
                    "You can enter the following levels of logging: debug, info, error]\n");
    fprintf(stderr, "[-n/--number_of_threads <number> - Enter the number of threads (the default number "
                    "is 10). The number of threads must be greater than 0]\n");
}

int main(int argc, char *argv[])
{
    dnstun_server_ret_t status;
    dnstun_server_t *dnstun_server;

    char *port = "8000";
    char *level = "info";
    int level_upto;
    int number_of_threads = 10;

    int opt;
    struct option options[5];

    options[0].name = "help";
    options[0].has_arg = no_argument;
    options[0].val = 'h';
    options[0].flag = NULL;

    options[1].name = "port";
    options[1].has_arg = required_argument;
    options[1].val = 'p';
    options[1].flag = NULL;

    options[2].name = "level";
    options[2].has_arg = required_argument;
    options[2].val = 'l';
    options[2].flag = NULL;

    options[3].name = "number_of_threads";
    options[3].has_arg = required_argument;
    options[3].val = 'n';
    options[3].flag = NULL;

    options[4].name = NULL;

    while ((opt = getopt_long(argc, argv, "hp:l:n:", options, NULL)) != -1)
    {
        switch(opt)
        {
            case 'h':
                s_print_usage(argv[0]);
                return 0;
            case 'p':
                port = optarg;
                break;
            case 'l':
                level = optarg;
                break;
            case 'n':
                number_of_threads = atoi(optarg);
                break;
            default:
                return -1;
        }
    }

    openlog(NULL, LOG_PERROR, LOG_USER);

    if(!strcmp(level, "debug"))
        level_upto = LOG_DEBUG;
    else if(!strcmp(level, "info"))
        level_upto = LOG_INFO;
    else if(!strcmp(level, "error"))
        level_upto = LOG_ERR;
    else
    {
        syslog(LOG_ERR, "The wrong level of logging - %s - is entered.", level);
        return -1;
    }

    setlogmask(LOG_UPTO(level_upto));

    if(number_of_threads < 1)
    {
        syslog(LOG_ERR, "The wrong number of threads - %d - is entered.", number_of_threads);
        return -1;
    }

    status = dnstun_server_init(&dnstun_server, port, 10);
    if(status != DNSTUN_SERVER_RET_OK)
        return -1;

    signal(SIGINT, s_stop_server);

    while(!stop)
        dnstun_server_poll(dnstun_server);

    dnstun_server_deinit(dnstun_server);
    closelog();
    return 0;
}

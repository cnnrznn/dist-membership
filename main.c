#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "channel.h"

void print_usage(void);

static char cont = 1;

void
handle_sigint(int sig)
{
        cont = 0;
}

int main(int argc, char **argv)
{
        char *port = NULL;
        char *hostfile = NULL;
        int id = -1;
        size_t timeout = 3;

        int opt;
        char options[] = { "h:p:i:t:" };

        int data;

	if (SIG_ERR == signal(SIGINT, handle_sigint)) {
                perror("Could not set signal handler");
                goto err;
        }

        while ((opt = getopt(argc, argv,  options)) != -1) {
                switch (opt) {
                        case 'h':
                                hostfile = optarg;
                                fprintf(stderr, "Hostfile is %s\n", hostfile);
                                break;
                        case 'p':
                                port = optarg;
                                fprintf(stderr, "Port is %s\n", port);
                                break;
                        case 'i':
                                id = atoi(optarg);
                                fprintf(stderr, "ID is %d\n", id);
                                break;
                        case 't':
                                timeout = atol(optarg);
                                fprintf(stderr, "timeout is %lu\n", timeout);
                                break;
                        default:
                                fprintf(stderr, "Unknown option, committing sepuku...\n");
                                goto err;
                }
        }

        if (NULL == port || NULL == hostfile || id < 0) {
                print_usage();
                goto err;
        }

        if (ch_init(hostfile, port, id, timeout))
                goto err;

        srand(time(NULL));

        while (cont) {
                ch_tick();
        }

        return 0;
err:
        return 1;
}

void
print_usage()
{
        // TODO implement
}

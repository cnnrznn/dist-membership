#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "channel.h"
#include "operation.h"
#include "queue.h"

#define HOSTS_MAX 1024

static int sk = -1;
static struct addrinfo hints, *skaddr;
static char *hosts[HOSTS_MAX];
static struct addrinfo *tmpaddr;
static struct sockaddr hostaddrs[HOSTS_MAX];
static size_t hostaddrslen[HOSTS_MAX];
static int nhosts = 0;
static int id = -1;
static size_t timeout;

static time_t last_heartbeat;
static time_t *hb_vec;
static queue *hb_q;
static queue *op_q;
static char *alive;

static uint32_t req_id = 0;

static char is_leader = 0;

int
ch_init(char *hostfile, char *port, int _id, size_t _timeout)
{
        FILE *f;
        size_t linelen = 32;
        ssize_t strlen;
        char *line;
        int i;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        id = _id;
        timeout = _timeout;

        // read hostfile
        f = fopen(hostfile, "r");
        if (NULL == f) {
                perror("Unable to open hostfile");
                goto err_open;
        }

        line = malloc(linelen);

        while ((strlen = getline(&line, &linelen, f)) > 1) {
                line[strlen-1] = '\0';
                hosts[nhosts] = malloc(strlen);
                memcpy(hosts[nhosts], line, strlen);

                if (getaddrinfo(hosts[nhosts], port, &hints, &tmpaddr)) {
                        perror("Unable to get target addr info for target addr");
                        goto err_addr;
                }
                hostaddrs[nhosts] = *(tmpaddr->ai_addr);
                hostaddrslen[nhosts] = tmpaddr->ai_addrlen;
                freeaddrinfo(tmpaddr);

                fprintf(stderr, "Read '%s(%lu:%lu)' from hostfile\n", hosts[nhosts], linelen, strlen);
                nhosts++;
        }

        free(line);
        fclose(f);

        // allocate socket
        fprintf(stderr, "I am %s\n", hosts[id]);
        if (getaddrinfo(hosts[id], port, &hints, &skaddr)) {
                perror("Unable to getaddrinfo()");
                goto err_addr;
        }
        if ((sk = socket(skaddr->ai_family, skaddr->ai_socktype, skaddr->ai_protocol)) == -1) {
                perror("Unable to create socket");
                goto err_addr;
        }
        if (bind(sk, skaddr->ai_addr, skaddr->ai_addrlen) == -1) {
                perror("Unable to bind socket");
                goto err_addr;
        }

        alive = calloc(nhosts, sizeof(char));
        alive[id] = 1;

        hb_vec = malloc(nhosts * sizeof(time_t));
        for (i=0; i<nhosts; i++)
                hb_vec[i] = time(NULL);

        hb_q = q_alloc(1024);
        op_q = q_alloc(1024);

        last_heartbeat = time(NULL);

        if (0 == id)
                is_leader = 1;

        return 0;
err_addr:
        freeaddrinfo(skaddr);
err_open:
        return -1;
}

int
ch_fini(void)
{
        close(sk);
        q_free(hb_q);

        return -1;
}

static void
drain_socket(void)
{
        char msg[MSGBUFLEN] = { 0 };
        struct sockaddr_storage from;
        int fromlen = sizeof(struct sockaddr_storage);
        int *type;
        HBMessage *hbm;

        while (recvfrom(sk, msg, MSGBUFLEN, MSG_DONTWAIT,
                        (struct sockaddr *)&from, &fromlen) > 0) {
                type = (int *)msg;
                fprintf(stderr, "Received message of type %d\n", *type);
                switch (*type) {
                case HB:
                        fprintf(stderr, "Draining HBMessage\n");
                        hbm = malloc(sizeof(HBMessage));
                        memcpy(hbm, msg, sizeof(HBMessage));
                        q_push(hb_q, hbm);
                        break;
                default:
                        // TODO some error handling
                        abort();
                }
        }
}

static void
heartbeat(void)
{
        int i;
        time_t currtime = time(NULL);
        HBMessage mybeat = {
                .type = HB,
                .id = id,
                };
        HBMessage *hbm;

        // send heartbeat
        if (difftime(currtime, last_heartbeat) > timeout) {
                for (i=0; i<nhosts; i++) {
                        fprintf(stderr, "Sending HBMessage to %d\n", i);
                        sendto(sk, &mybeat, sizeof(HBMessage), 0, &hostaddrs[i], hostaddrslen[i]);
                }

                last_heartbeat = currtime;
        }

        // process heartbeat queue
        while (hbm = q_pop(hb_q)) {
                fprintf(stderr, "Received HBMessage from %d\n", hbm->id);
                hb_vec[hbm->id] = time(NULL);

                // if alive[hbm->id] == 0, set to 1 push JOIN op to op_q
                if (is_leader && 0 == alive[hbm->id]) {
                        alive[hbm->id] = 1;

                        q_push(op_q, new_op(JOIN, hbm->id, nhosts));
                }

                free(hbm);
        }

        // detect dead peer
        for (i=0; i<nhosts; i++) {
                if (difftime(currtime, hb_vec[i]) > 2*timeout) {
                        fprintf(stdout, "Peer %d not reachable\n", i);

                        // if alive[i] == 1 set to 0 and push LEAVE to op_q
                        if (is_leader && 1 == alive[i]) {
                                alive[i] = 0;

                                q_push(op_q, new_op(LEAVE, i, nhosts));
                        }
                }
        }
}

static void
process_op_q(void)
{
        Operation *op;

        while (op = q_peek(op_q)) {
                switch (op->type) {
                case JOIN:
                        break;
                default:
                        abort();
                }
        }
}

void
ch_tick(void)
{
        // flush all incoming messages
        drain_socket();

        // process heartbeats
        heartbeat();

        // do membership protocol
        process_op_q();
}

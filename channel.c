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
#define TIMEOUT_FACTOR 2

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
static int nalive = 1;
static char *warm;

static uint32_t req_id = 0;
static uint32_t view_id = 0;

static char leader = 0;

static PendingOp pendop;

static void
print_view(void)
{
        int i;

        fprintf(stdout, "View %u: [", view_id);
        for (i=0; i<nhosts; i++)
                fprintf(stdout, "%d, ", alive[i]);
        fprintf(stdout, "]\n");
}

static void
print_op_q(void)
{
        int i;
        Operation *op;

        fprintf(stderr, "OPQ: {");
        for (i=0; i<op_q->n; i++) {
                op = op_q->arr[i];
                switch (op->type) {
                case JOIN:
                        fprintf(stderr, "JOIN: %d, ", op->pid);
                        break;
                case LEAVE:
                        fprintf(stderr, "LEAVE: %d, ", op->pid);
                        break;
                }
        }
        fprintf(stderr, "}\n");
}

static void
process_reqm(ReqMessage *reqm)
{
        OkMessage okm;

        if (reqm->view_id != view_id)
                return; // don't process requests for different view

        // save pending request
        pendop.op_id = reqm->req_id;
        pendop.view_id = reqm->view_id;
        pendop.type = reqm->op;
        pendop.pid = reqm->pid;

        // respond with OK
        okm.type = OK;
        okm.req_id = reqm->req_id;
        okm.view_id = view_id;
        okm.recipient = id;

        sendto(sk, &okm, sizeof(OkMessage), 0, &hostaddrs[leader], hostaddrslen[leader]);
}

static void
process_nvm(NewVMessage *nvm)
{
        int i;
        OkMessage okm;
        char *arr = ((char*)nvm) + sizeof(NewVMessage);

        if (view_id < nvm->view_id) {
                for (i=0; i<nhosts; i++)
                        alive[i] = arr[i];

                view_id = nvm->view_id;
                print_view();
        }

        okm.type = OK;
        okm.req_id = nvm->req_id;
        okm.view_id = view_id;
        okm.recipient = id;

        sendto(sk, &okm, sizeof(OkMessage), 0, &hostaddrs[leader], hostaddrslen[leader]);
}

static void
process_okm(OkMessage *okm)
{
        Operation *op;

        if (NULL == (op = q_peek(op_q)))
                return; // no pending operation
        if (okm->req_id != op->op_id || okm->view_id != view_id)
                return; // ack for different operation

        if (0 == op->acks[okm->recipient]) {
                op->acks[okm->recipient] = 1;
                op->nacks++;

                op->timeouts[okm->recipient] = 2;
                op->timers[okm->recipient] = 0;

                if (op->nacks >= nalive) {      // "turn on" the new process
                        view_id++;
                        op->nacks++;
                        op->acks[op->pid] = 1;
                        if (JOIN == op->type) {
                                alive[op->pid] = 1;
                                nalive++;

                                warm[op->pid] = 0;
                        }

                        print_view();
                }
        }
        else if (op->nacks >= nalive && 0 == op->facks[okm->recipient]) {
                op->facks[okm->recipient] = 1;
                op->nfacks++;
        }
}

static void
drain_socket(void)
{
        char msg[MSGBUFLEN] = { 0 };
        struct sockaddr_storage from;
        int fromlen = sizeof(struct sockaddr_storage);
        int *type;
        HBMessage *hbm;
        PendingOp pop;
        ReqMessage *reqm;
        NewVMessage *nvm;

        while (recvfrom(sk, msg, MSGBUFLEN, MSG_DONTWAIT,
                        (struct sockaddr *)&from, &fromlen) > 0) {
                type = (int *)msg;
                fprintf(stderr, "Received message of type %d\n", *type);
                switch (*type) {
                case HB:
                        fprintf(stderr, "\tDraining HBMessage\n");
                        hbm = malloc(sizeof(HBMessage));
                        memcpy(hbm, msg, sizeof(HBMessage));
                        q_push(hb_q, hbm);
                        break;
                case OK:
                        fprintf(stderr, "\tDraining OkMessage\n");
                        process_okm((OkMessage *)msg);
                        break;
                case REQ:
                        fprintf(stderr, "\tDraining ReqMessage\n");
                        process_reqm((ReqMessage *)msg);
                        break;
                case NEWVIEW:
                        fprintf(stderr, "\tDraining NewVMessage\n");
                        process_nvm((NewVMessage *)msg);
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
                        //fprintf(stderr, "Sending HBMessage to %d\n", i);
                        sendto(sk, &mybeat, sizeof(HBMessage), 0, &hostaddrs[i], hostaddrslen[i]);
                }

                last_heartbeat = currtime;
        }

        // process heartbeat queue
        while (hbm = q_pop(hb_q)) {
                //fprintf(stderr, "Received HBMessage from %d\n", hbm->id);
                hb_vec[hbm->id] = time(NULL);

                // if alive[hbm->id] == 0, set to 1 push JOIN op to op_q
                if (id == leader && 0 == alive[hbm->id] && 0 == warm[hbm->id]) {
                        q_push(op_q, new_op(JOIN, hbm->id, nhosts));
                        warm[hbm->id] = 1;
                }

                free(hbm);
        }

        // detect dead peer
        for (i=0; i<nhosts; i++) {
                if (difftime(currtime, hb_vec[i]) > 2*timeout) {
                        fprintf(stdout, "Peer %d not reachable\n", i);

                        // if alive[i] == 1 set to 0 and push LEAVE to op_q
                        if (id == leader && 1 == alive[i]) {
                                alive[i] = 0;
                                nalive--;

                                q_push(op_q, new_op(LEAVE, i, nhosts));
                        }
                }
        }
}

static void
send_req(Operation *op)
{
        int i;
        time_t currtime = time(NULL);
        ReqMessage rm = {
                .type = REQ,
                .req_id = op->op_id,
                .view_id = view_id,
                .op = op->type,
                .pid = op->pid,
        };
        NewVMessage nvm = {
                .type = NEWVIEW,
                .view_id = view_id,
                .req_id = op->op_id,
        };
        char buf[MSGBUFLEN] = { 0 };

        for (i=0; i<nhosts; i++) {
                if (0 == alive[i])
                        continue;               // not in the group
                if (op->timeouts[i] > difftime(currtime, op->timers[i]))
                        continue;               // timeout not expired

                op->timeouts[i] *= TIMEOUT_FACTOR;
                op->timers[i] = currtime;

                if (op->nacks < nalive) {
                        if (1 == op->acks[i])
                                continue;       // already have ack
                        sendto(sk, &rm, sizeof(ReqMessage), 0, &hostaddrs[i], hostaddrslen[i]);
                }
                else if (op->nfacks < nalive) {
                        if (1 == op->facks[i])
                                continue;       // already have fack
                        memcpy(buf, &nvm, sizeof(NewVMessage));
                        memcpy(buf+sizeof(NewVMessage), alive, nhosts*sizeof(char));
                        sendto(sk, &buf, MSGBUFLEN*sizeof(char), 0, &hostaddrs[i], hostaddrslen[i]);
                }
        }
}

static void
process_op_q(void)
{
        Operation *op;

        if (NULL == (op = q_peek(op_q)))
                return;

        if (op->nfacks < nalive) {
                send_req(op);
        }
        else {
                q_pop(op_q);
                free_op(op);
        }
}

void
ch_tick(void)
{
        // flush all incoming messages
        drain_socket();

        // process heartbeats
        heartbeat();

        // DEBUG
        print_op_q();
        print_view();

        // do membership protocol
        process_op_q();

        fflush(stdout);
        fflush(stderr);
}

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

        warm = calloc(nhosts, sizeof(char));

        hb_vec = malloc(nhosts * sizeof(time_t));
        for (i=0; i<nhosts; i++)
                hb_vec[i] = time(NULL);

        hb_q = q_alloc(1024);
        op_q = q_alloc(1024);

        last_heartbeat = time(NULL);

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

#ifndef _OPS_H
#define _OPS_H

#define JOIN    1
#define LEAVE   2

typedef struct {
        int type;
        int pid;

        time_t *timeouts;
        char *acks;
        char *facks;
        int nacks, nfacks;
} Operation;

Operation *new_op(int _type, int _pid, int n);

#endif /* _OPS_H */

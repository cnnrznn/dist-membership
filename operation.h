#ifndef _OPS_H
#define _OPS_H

#include <stdint.h>

#define JOIN    1
#define LEAVE   2

typedef struct {
        int type;
        int pid;

        uint32_t op_id;
        double *timeouts;
        time_t *timers;
        char *acks;
        char *facks;
        int nacks, nfacks;
} Operation;

Operation *new_op(int _type, int _pid, int n);
void free_op(Operation *);

#endif /* _OPS_H */

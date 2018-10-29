#ifndef _OPS_H
#define _OPS_H

#include <stdint.h>

#define JOIN    1
#define LEAVE   2

typedef struct {
        int type;
        int pid;
        uint32_t op_id;
        char transition;

        double *timeouts;
        time_t *timers;
        char *acks;
        char *facks;
        int nacks, nfacks;
} Operation;

typedef struct {
        int valid;                      // is there a current pending operation?
        uint32_t op_id;
        uint32_t view_id;
        int type;
        int pid;
} PendingOp;

Operation *new_op(int _type, int _pid, int n);
void free_op(Operation *);

#endif /* _OPS_H */

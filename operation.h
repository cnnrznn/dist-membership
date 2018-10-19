#ifndef _OPS_H
#define _OPS_H

#include <stdint.h>

#define JOIN    1
#define LEAVE   2

typedef struct {
        int type;
        int pid;
        uint32_t op_id;
        uint32_t view_id;

        double *timeouts;
        time_t *timers;
        char *acks;
        char *facks;
        int nacks, nfacks;
} Operation;

typedef struct {
        uint32_t op_id;
        uint32_t view_id;
        int type;
        int pid;
} PendingOp;

Operation *new_op(int _type, int _pid, int n, uint32_t _view_id);
void free_op(Operation *);

#endif /* _OPS_H */

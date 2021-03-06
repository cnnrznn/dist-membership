#include <stdlib.h>

#include "operation.h"

static uint32_t op_id = 0;

Operation *
new_op(int _type, int _pid, int n)
{
        int i;
        Operation *op = malloc(sizeof(Operation));

        op->type = _type;
        op->pid = _pid;
        op->op_id = ++op_id;
        op->transition = 0;

        op->timeouts = calloc(n, sizeof(double));
        for (i=0; i<n; i++)
                op->timeouts[i] = 2;

        op->timers = calloc(n, sizeof(time_t));
        op->acks = calloc(n, sizeof(char));
        op->facks = calloc(n, sizeof(char));
        op->nacks = 0;
        op->nfacks = 0;

        return op;
}

void
free_op(Operation *op)
{
        free(op->timeouts);
        free(op->timers);
        free(op->acks);
        free(op->facks);
        free(op);

        return;
}

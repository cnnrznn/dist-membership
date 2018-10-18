#include <stdlib.h>

#include "operation.h"

Operation *
new_op(int _type, int _pid, int n)
{
        Operation *op = malloc(sizeof(Operation));

        op->type = _type;
        op->pid = _pid;

        op->timeouts = calloc(n, sizeof(double));
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

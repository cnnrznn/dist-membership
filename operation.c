#include <stdlib.h>

#include "operation.h"

Operation *
new_op(int _type, int _pid, int n)
{
        Operation *op = malloc(sizeof(Operation));

        op->type = _type;
        op->pid = _pid;
        op->timeouts = malloc(n * sizeof(time_t));
        op->acks = malloc(n * sizeof(char));
        op->facks = malloc(n * sizeof(char));
        op->nacks = 0;
        op->nfacks = 0;

        return op;
}

#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

static void
swap(int a, int b, queue *q)
{
        void *tmp = q->arr[a];
        q->arr[a] = q->arr[b];
        q->arr[b] = tmp;
}

static int
partition(int lo, int hi, queue *q)
{
        void *pivot = q->arr[hi];
        int i, lohi=lo;

        for (i=lo; i<hi; i++) {
                if (q->comp(q->arr[i], pivot) > 0) {
                        swap(i, lohi++, q);
                }
        }
        swap(hi, lohi, q);

        return lohi;
}

static void
quicksort(int lo, int hi, queue *q)
{
        int lohi;

        if (hi < lo) {
                return;
        }

        lohi = partition(lo, hi, q);

        quicksort(lo, lohi-1, q);
        quicksort(lohi+1, hi, q);
}

static void*
binsearch(queue *q, void *other, int start, int end)
{
        char res;
        int first = start, last = end;
        int index = (first + last) / 2;

        if (last < first)
                return NULL;

        while ((res = q->comp(q->arr[index], other)) != 0) {
                if (res > 0)
                        first = index + 1;
                else
                        last = index - 1;

                index = (first + last) / 2;

                if (last < first)
                        return NULL;
        }

        return q->arr[index];
}

int
q_push(queue* q, void* e)
{
        if (q->size == q->n + 1) {
                q->size *= 2;
                q->arr = realloc(q->arr, q->size*sizeof(void*));
        }

        q->arr[q->n++] = e;
}

void*
q_pop(queue* q)
{
        void* ret;
        int i;

        if (NULL == q || q->n <= 0)
                return NULL;

        ret = q->arr[0];

        for (i=0; i<q->n-1; i++)
                q->arr[i] = q->arr[i+1];

        q->n--;

        return ret;
}

void*
q_peek(queue* q)
{
        if (NULL == q || q->n <= 0)
                return NULL;

        return q->arr[0];
}

void*
q_search(queue *q, void *other, char (*comp)(void*,void*))
{
        q->comp = comp;
        binsearch(q, other, 0, q->n-1);
}

void
q_sort(queue *q, char (*comp)(void*,void*))
{
        q->comp = comp;
        quicksort(0, q->n-1, q);
}

queue*
q_alloc(size_t size)
{
        queue* q = malloc(sizeof(queue));

        q->arr = calloc(size, sizeof(void*));
        q->size = size;
        q->n = 0;

        return q;
}

void
q_free(queue* q)
{
        free(q->arr);
        free(q);
}

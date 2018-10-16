#ifndef _QUEUE_H
#define _QUEUE_H

typedef struct {
        void **arr;
        size_t size;
        size_t n;
        char (*comp) (void*, void*);
} queue;

queue* q_alloc(size_t);
void q_free(queue*);

int q_push(queue*, void*);
void* q_pop(queue*);
void* q_peek(queue*);

void q_sort(queue*, char (*comp)(void*,void*));
void *q_search(queue*, void*, char (*comp)(void*,void*));

#endif /*_QUEUE_H */

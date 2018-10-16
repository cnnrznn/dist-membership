#ifndef _MSG_H
#define _MSG_H

#include <stdint.h>

typedef struct {
        uint32_t type;                          // 1
        uint32_t sender;
        uint32_t msg_id;
        uint32_t data;
} DataMessage;

typedef struct {
        uint32_t type;                          // 2
        uint32_t sender;
        uint32_t msg_id;
        uint32_t proposed_seq;
        uint32_t proposer;
} AckMessage;

typedef struct {
        uint32_t type;                          // 3
        uint32_t sender;
        uint32_t msg_id;
        uint32_t final_seq;
        uint32_t final_seq_proposer;
} SeqMessage;

typedef struct {
        uint32_t type;                          // 4
        uint32_t sender;
        uint32_t msg_id;
        uint32_t proposer;
} FinMessage;

typedef struct {
        uint32_t type;                          // 5
        uint32_t initiator;
        uint32_t ckpt_id;
} CkptMessage;

typedef struct {
        uint32_t type;                          // 6
        uint32_t initiator;
        uint32_t ckpt_id;
        uint32_t recipient;
} CkptAck;

#endif /* _MSG_H */

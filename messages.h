#ifndef _MSG_H
#define _MSG_H

#include <stdint.h>

#define MSGBUFLEN 1024

#define HB              1
#define REQ             2
#define NEWVIEW         3

typedef struct {
        int type;                       // 1
        int id;
} HBMessage;

typedef struct {
        int type;
        uint32_t req_id;
        uint32_t view_id;
        int op;
        int pid;
} ReqMessage;

typedef struct {
        int type;
        uint32_t view_id;
} NewVMessage;

#endif /* _MSG_H */

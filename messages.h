#ifndef _MSG_H
#define _MSG_H

#define MSGBUFLEN 1024

#define HB              1
#define REQ             2

typedef struct {
        int type;                       // 1
        int id;
} HBMessage;

typedef struct {
        int type;
        int id;
} JoinMessage;

typedef struct {
        int type;
        int id;
} LeaveMessage;

typedef struct {
        int type;
        int req_id;
        int view_id;
        int op;
        int newmember;
} ReqMessage;

#endif /* _MSG_H */

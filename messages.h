#ifndef _MSG_H
#define _MSG_H

#define MSGBUFLEN 1024

#define HB              1
#define JOIN            2
#define LEAVE           3

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

#endif /* _MSG_H */

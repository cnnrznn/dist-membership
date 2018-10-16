#ifndef _MSG_H
#define _MSG_H

#define MSGBUFLEN 1024

typedef struct {
        int type;                       // 1
        int id;
} HBMessage;

#endif /* _MSG_H */

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include "messages.h"

extern char special_flag;

int ch_init(char *, char *, int, size_t);
int ch_fini(void);

void ch_tick(void);

#endif /* _CHANNEL_H */

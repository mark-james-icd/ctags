#ifndef _UTILS_H_
#define _UTILS_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

//#include "lightring_animation.h"

//#include "packet_arbiter.h"
//#include "rcu_api.h"
//#include "event.h"
//#include "server.h"

#define TRACE(level, fmt, ...) \
	do { if (g_debug && level>0) printf(fmt, __VA_ARGS__); } while (0);

#define UNUSED(x) (void)x

extern char pkt_label[0x2F][32];
extern size_t pkt_size[0x2F];



// function prototypes

size_t GET_PKT_SIZE(char * data);
void DUMP_PKT(char * data);
size_t min(size_t size1, size_t size2);
void WAIT_PROMPT(void);
void dbg(int level, const char * format, ...);

#endif /* UTILS_H */


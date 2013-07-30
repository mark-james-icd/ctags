#ifndef _PACKET_MUX_H_
#define _PACKET_MUX_H_

#include <serial_port.h>
#include <cobs.h>
#include <string.h>
#include <stddef.h>

//#include "packet_arbiter.h"
//#include "rcu_api.h"

#define PKT_TYPE_SET_LIGHTRING  0x01

#define WAIT_READ()	usleep(20000);
#define WAIT_WRITE()	usleep(20000);

bool packet_mux_init(bool debug);
void packet_mux_deinit(void);

uint16_t packet_mux_receive(char * pkt, size_t * len_resp);
void packet_mux_request(char * pkt, size_t * len, bool response_required);


#endif /* PACKET_MUX_H */

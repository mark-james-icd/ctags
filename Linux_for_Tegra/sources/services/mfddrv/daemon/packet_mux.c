#include <stdbool.h>
#include <packet_arbiter.h>
#include <rcu_api.h>
#include <packet_mux.h>
#include <time.h>
#include <pthread.h> 
#include <sys/types.h>
#include <event.h>
#include <poll.h>

#include "utils.h"

extern bool g_debug;
pthread_mutex_t comm_mutex = PTHREAD_MUTEX_INITIALIZER;

extern pthread_mutex_t ser_mutex;
extern pthread_mutex_t request_mutex;
extern pthread_mutex_t response_mutex;
extern custom_event response_event;

char resp_buffer[2048];
arbiter_pkt * g_response_pkt = (arbiter_pkt *)resp_buffer;
size_t        g_response_pkt_len = sizeof(resp_buffer);
struct pollfd g_serfd [1];
extern int g_fd;


//
// Initialize the packet_mux functions
//

bool packet_mux_init(bool debug)
{
	char dev[30];
	g_debug = debug;
	char * c = dev;
	//int res;

	// TODO: Abstract HW

	// using default port
	// TODO: verify correct hardware, check other ports
	strcpy(dev, "/dev/ttyUSB0");
	TRACE(1, "No serial port specified, defaulting to %s\n", dev);

	serial_port_init(dev, B115200);

	if (serial_port_open() != 0) {
		TRACE(1, "Error opening serial port (%d)\n", 0);
		TRACE(1, "Enter serial port name (%d):\n", 0);

		bzero(dev, sizeof(dev));
		c = fgets(dev, sizeof(dev) ,stdin);
		UNUSED(c);

		serial_port_init(dev, B115200);
		if (serial_port_open() != 0) {
			TRACE(1, "Error opening serial port (%d)\n", 0);
			return false;
		}

/*
		strcpy(dev, "/dev/usbdev2.8");
		TRACE(1, "Attempt opening %s\n", dev);
		serial_port_init(dev, B115200);
		if (serial_port_open() != 0) {
			TRACE(1, "Error opening serial port (%d)\n", 0);
			strcpy(dev, "/dev/usbdev2.6");
			TRACE(1, "Attempt opening %s\n", dev);
			serial_port_init(dev, B115200);
			if (serial_port_open() != 0) {
				TRACE(1, "Error opening serial port (%d)\n", 0);
				return false;
			}
		}
*/
	}

	// this disables the microcontrollers 'reset' line
	serial_port_set_dtr(false);
	usleep(1000);

	g_serfd[0].fd = g_fd;
	g_serfd[0].events = POLLIN;	// serial data to be read
	g_serfd[0].revents = 0;

	return true;
}


void packet_mux_deinit()
{
	serial_port_set_dtr(true);
}


/*
    Receive response to a request.
*/

uint16_t packet_mux_receive(char * pkt, size_t * len_resp)
{
	uint16_t ret;
	uint8_t cobs_pkt[100] = {};
	size_t cobs_res = 0;
	uint8_t buff[2048];
	arbiter_pkt * arb_pkt;
	eeprom_packet *epkt;
int i;	

	TRACE(0, "packet_mux_receive: attempt ser_mutex LOCK (%x)\n", (unsigned int)pthread_self());
        pthread_mutex_lock(&ser_mutex);
	TRACE(0, "packet_mux_receive: ser_mutex LOCKED (%x)\n", (unsigned int)pthread_self());

	// read and decode response into temporary buffer
	memset(cobs_pkt, 0, sizeof(cobs_pkt));
	memset(pkt, 0, *len_resp);
	memset(buff, 0, 2048);

	// poll for serial data available to read (returns 0 if timed out, -1 if
	// error, positive number indicates how many structures have events).
	ret = poll(g_serfd, 1, 50);	// wait for minimum of 50 msec
	if (ret > 0) {
		usleep(20000);
		WAIT_READ();	
		ret = serial_port_read((char *)cobs_pkt, sizeof(cobs_pkt));
		if (ret == -1) {
			TRACE(1, "nothing to see here (%x)\n", ret);
			goto cleanup;
		}
		if (ret > (int)sizeof(cobs_pkt)) {
			TRACE(1, "Read to much data! (%d)\n", (int)sizeof(cobs_pkt));
			goto cleanup;
		}

		// decode response
		cobs_res = cobs_decode((uint8_t *)&cobs_pkt, ret, buff);

		// TODO: there seems to be an issue that prod_id is coming
		// back not encoded, for now if decoding fails, assume its
		// just not encoded. Expect in this case there is a lead byte that 
		// may need to be removed.

		if (cobs_res == 0) {
			memcpy(buff, &cobs_pkt[1], ret-1);
			cobs_res = ret-1;	
		}

		// copy response into caller's buffer
		if (cobs_res < *len_resp)
			*len_resp = cobs_res;


		arb_pkt = (arbiter_pkt *)buff;
		if (arb_pkt->pkt_type > 0x0 && arb_pkt->pkt_type <= 0x0E) {
			TRACE(0, "packet_mux_receive: copying response data to buffer (%d)\n", (int)*len_resp);
			memcpy(pkt, buff, *len_resp);
		}

		TRACE(0, "packet_mux_receive: got response: type=%x, data=%x %x %x (%d))\n", 
			arb_pkt->pkt_type, arb_pkt->data[0], arb_pkt->data[1], arb_pkt->data[2], (int)*len_resp);

		if (arb_pkt->pkt_type==0xa || arb_pkt->pkt_type==0xd) {
			epkt = (eeprom_packet *)pkt;
			TRACE(0, "Data: %x %x %x %x %x %x %x %x (%d))\n", 
				epkt->i2c_cmd.data[0], epkt->i2c_cmd.data[1], epkt->i2c_cmd.data[2], 
				epkt->i2c_cmd.data[3], epkt->i2c_cmd.data[4], epkt->i2c_cmd.data[5], 
				epkt->i2c_cmd.data[6], epkt->i2c_cmd.data[7], 
	 			(int)*len_resp);
		}
	} else if (ret == 0) {
		; //printf("poll timed out\n");
	} else {
		; //printf("poll error\n");
	}

cleanup:
        pthread_mutex_unlock(&ser_mutex);
	TRACE(0, "packet_mux_receive: ser_mutex UNLOCKED (%x)\n", (unsigned int)pthread_self());

	return ret;
}


/*
    Send a request, optionally wait for response.
*/

void packet_mux_request(char * pkt, size_t * len, bool response_required)
{
	uint8_t cobs_pkt[100] = {};
	size_t cobs_res = 0;
	size_t newlen, enc_len;
	int res;

	TRACE(0, "+packet_mux_request (%d)\n", (int)*len);

	// aquire the request lock so that no other requests can happen until this one is completed
	pthread_mutex_lock(&request_mutex);
	TRACE(0, "packet_mux_request: Acquired request mutex (%x)\n", (unsigned int)pthread_self());

	// encode the request
	enc_len = *len;
	cobs_res = cobs_encode((uint8_t *)pkt, enc_len, cobs_pkt);	

	// now aquire the communication lock so we don't interleave reads and writes
	pthread_mutex_lock(&ser_mutex);
	TRACE(0, "packet_mux_request: Acquired communication mutex (%x)\n", (unsigned int)pthread_self());

	// now safe to write the request to the hardware
	res = serial_port_write((char *)cobs_pkt, cobs_res + 1);

	// release the communication lock
	pthread_mutex_unlock(&ser_mutex);
	TRACE(0, "packet_mux_request: Released communication mutex (%x)\n",
		(unsigned int)pthread_self());

	// wait for the response
        event_wait(&response_event);
	event_reset(&response_event);
	TRACE(0, "packet_mux_request: Reponse event signaled and reset (%d)\n", (int)*len);
	
	// lock access to the response pkt
	pthread_mutex_lock(&response_mutex);
	TRACE(0, "packet_mux_request: Acquired response mutex (%d, %d)\n", (int)*len, (int)g_response_pkt_len);

	if (response_required) {

		// verify correct response packet type and copy to caller's buffer
//		if (g_response_pkt->pkt_type == pkt[0]) {
			newlen = min(*len, g_response_pkt_len);
			*len = newlen;
			memcpy(pkt, g_response_pkt, *len);
			TRACE(0, "packet_mux_request: RESPONSE: %x %x %x %x %x %x %x %x (%d)\n", 
				pkt[0], pkt[1], pkt[2], pkt[3], pkt[4], pkt[5], pkt[6], pkt[7],
				(int)*len);
//		} else {
//			printf("packet_mux_request: RESPONSE: invalid type (%x, should be %x)\n", 
//				g_response_pkt->pkt_type, pkt[0]);
//		}
	}
		// release the response mutex
		pthread_mutex_unlock(&response_mutex);
		TRACE(0, "packet_mux_request: Released response mutex (%d)\n", (int)*len);
	
	//} else {
		// most set commands don't return anything other than a status
		// packet that isn't generally useful as it's returned
		// immediately (most errors would come back as error packet
		// rather than an error in the status packet). So don't wait 
		// for the actual response, in this case.
	//}

	// release the request lock
	pthread_mutex_unlock(&request_mutex);
	TRACE(0, "-packet_mux_request: unlocked request mutex (%d)\n", (int)*len);
	return;
}


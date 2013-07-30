/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

/* Copyright (c) 2013 iCD
   This function listens for and parses the api requests sent
   via a socket interface. It communicates with the packet_mux
   functions and other submodules to carry out the requests.
*/

/* linux include files */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

/* custom include files */

#include <packet_arbiter.h>
#include <rcu_api.h>
#include <lightring_animation.h>
#include "packet_mux.h"
#include "event.h"
#include "server.h"
#include "utils.h"

#define MFDDRV_VERSION "0.1.3"

/* global variables */
int g_newsockfd = 0;
int g_sockfd = 0;
extern bool g_debug;
extern bool g_halt_requested;

int thread_flag;
pthread_cond_t thread_flag_cv;
pthread_mutex_t thread_flag_mutex;

extern custom_event lr_event;
extern custom_event flasher_event;
extern custom_event running_event;
flasher_seq_packet g_flasher_seq_pkt;



typedef struct lr_temp_packet {
	uint8_t type;
	lr_led leds[NUM_LR_LEDS+2];
}__attribute__((packed)) lr_temp_packet;


/*
   Initialize the socket interface used to receive API requests.
*/

bool socket_if_init(void)
{
    socklen_t clilen;
    struct sockaddr_un serv_addr, cli_addr;
    int res;

    // Create the server side of the socket (a communication endpoint for
    // local, stream based communication using the default stream protocol).

    g_sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (g_sockfd < 0) {
        printf("ERROR opening socket\n");
	return false;
    }

    // bind the socket to an address (a port number on the host machine)

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_LOCAL;
    strncpy(serv_addr.sun_path, SOCKET_IF_PATH, sizeof(serv_addr.sun_path) - 1);

    res = bind(g_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (res < 0) {
        printf("ERROR on binding (%d)\n", res);
	return false;
    }

    // listen for socket connections (default is up to five connections waiting).

    listen(g_sockfd, 5);

    // Block until a client connects.

    printf("Waiting for client to connect...\n");
    clilen = sizeof(cli_addr);
    g_newsockfd = accept(g_sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (g_newsockfd < 0) {
        printf("ERROR on accept\n");
	return false;
    }
    printf("Client connected...\n");

    return true; 
}


/*
   Deinitialize the socket.
*/

void socket_if_deinit()
{
    printf("Unbind from socket...\n");

    if (g_newsockfd != 0) {
        close(g_newsockfd);
        g_newsockfd = 0;
    }
    if (g_sockfd != 0) {
        close(g_sockfd);
        g_sockfd = 0;
    }
    unlink(SOCKET_IF_PATH);

    return;
}


/*
   Read a request from the socket
*/

uint16_t socket_if_getreq(char * data, int datasize)
{
	int n;

	bzero(data, datasize);
	n = read(g_newsockfd, data, datasize);
	if (n <= 0) {
		printf("client pipe disconnected\n");
	}

	return n;
}


/*
   Write a response back to the socket
*/

uint16_t socket_if_sendresp(char * data, int datasize)
{
	int n;

	//printf("SEND RESPONSE TO SOCKET: %x %x %x (%d)\n", data[0], data[1], data[2], datasize);
	n = write(g_newsockfd, data, datasize);
	if (n < 0) {
		printf("ERROR writing to socket\n");
	}

	return n;
}


/*
   This funciton parses and the request based on request type, and passes it
   on to the appropriate submodule if necessary otherwise sends it to the 
   hardware communications channel using the packet_mux functions.
*/

int parse_req(char * reqdata, int datalen)
{
	arbiter_pkt     * pkt_hdr;
	bool animate = false;
	lr_animation_pattern_packet * pattern_pkt;
	eeprom_packet  * eeprom_pkt;
	eeprom_packet epkt;
	drv_version_packet * drv_ver_pkt;
	size_t len = (size_t)datalen;
	lr_temp_packet lrpkt;
	status_led_packet * ledpkt;
	int i;

	packet_arbiter_status status_pkt;
	status_pkt.cmd_type = PACKET_TYPE_STATUS;
	status_pkt.err_code = 0;

	// validate input parameters
	if (reqdata == NULL || datalen == 0)
		return 0;

	// process request based on type
	pkt_hdr = (arbiter_pkt *)reqdata;
	switch (pkt_hdr->pkt_type) {

	case PACKET_TYPE_STATUS:
		break;

	case PACKET_TYPE_GET_REMOTE:
		// TODO: Not implemented yet
		DUMP_PKT((char *)reqdata);
		break;

	case PACKET_TYPE_ERROR:                 // 0x00
		// TODO: Not implemented yet
		DUMP_PKT((char *)reqdata);
		break;

	case PACKET_TYPE_SET_LEDS:              // 0x02	
		printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		// reset the animation event to stop any active animations
		event_reset(&lr_event);

		// TODO: **** temporarily using lr 8 and 9 for icon LEDs ****
		ledpkt = (status_led_packet *)reqdata;
		lrpkt.type = PACKET_TYPE_SET_LIGHTRING;
		for (i=0; i < NUM_LR_LEDS+2; i++) {
			lrpkt.leds[i].color = 0;
			lrpkt.leds[i].brightness = LED_BRIGHTNESS_HIGH4;
		}
		if (ledpkt->data.led_id == LED_ID_RCU_SIGNMAIL && ledpkt->data.intensity != 0)
			lrpkt.leds[NUM_LR_LEDS].color = LED_COLOR_WHITE;
		else if (ledpkt->data.led_id == LED_ID_RCU_MISSEDCALL && ledpkt->data.intensity != 0)
			lrpkt.leds[NUM_LR_LEDS+1].color = LED_COLOR_WHITE;
		else
			printf("no match on led id (%d)\n", ledpkt->data.led_id);

		// submits the request to the hardware port, don't wait for response
		len = sizeof(lrpkt);
		packet_mux_request((char *)&lrpkt, &len, false);
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_GET_TEMP:              // 0x03
	case PACKET_TYPE_GET_ALS:		// 0x0B
	case PACKET_TYPE_GET_PIR:		// 0x0E
	case PACKET_TYPE_GET_IR:                // 0x04
	case PACKET_TYPE_GET_SN:                // 0x05
	case PACKET_TYPE_GET_PRODUCT_ID:        // 0x07
	case PACKET_TYPE_GET_FW_VERSION:        // 0x09
		printf("MFDDRV: Processing %s request...(%d,%d)\n", pkt_label[pkt_hdr->pkt_type], datalen, (int)len);
		// submits the request to the hardware port, returns the response
		packet_mux_request(reqdata, &len, true);
		//DUMP_PKT(reqdata);
		// send the response back to the caller (this is a get command)
		socket_if_sendresp(reqdata, pkt_size[pkt_hdr->pkt_type]);
		break;

	case PACKET_TYPE_SET_BRIGHTNESS:        // 0x06
		printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		DUMP_PKT(reqdata);
		// submits the request to the hardware port, don't wait for response
		packet_mux_request(reqdata, &len, false);
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;
    
	case PACKET_TYPE_SET_LIGHTRING:         // 0x01
	case PACKET_TYPE_SET_LIGHTRING_RGB:     // 0x08
		printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		// reset the animation event to stop any active animations
		event_reset(&lr_event);
		DUMP_PKT(reqdata);
		// submits the request to the hardware port, don't wait for response
		packet_mux_request(reqdata, &len, false);
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_SET_LRANIMATION:       	// 0x80
	case PACKET_TYPE_SET_LRANIMATION_RGB:  		// 0x81
		//printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		printf("MFDDRV: Processing LR animation request\n");
		// reset the animation event to stop any active animations
		event_reset(&lr_event);
		// save the animation frames so we can continue running
		// the animation after the call returns.
		lightring_animator_parse(reqdata, datalen, &animate);
		// if this is a new animation request (and not a clear animation
		// request, then signal the event to start the animation.
		if (animate) {
			event_signal(&lr_event);
		}
		// lightring animation is inherently asyncronous (actual 
		// request happens through the animation thread, not directly 
		// from the caller), so just return a good status packet in
		// response to this call.
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_SET_LRANIMATION_PATTERN:	// 0x82
		printf("MFDDRV: Processing LR pattern animation request\n");
		// reset the animation event to stop any active animations
		event_reset(&lr_event);
		// save the animation frames corresponding to the predefined
		// pattern id we can continue running the animation after the
		// call returns.
		pattern_pkt = (lr_animation_pattern_packet *)reqdata;
		lightring_animator_parse_pattern(pattern_pkt->pattern_id, &animate);
		if (animate)
			event_signal(&lr_event);
		// lightring animation is inherently asyncronous (actual 
		// request happens through the animation thread, not directly 
		// from the caller), so just return a good status packet in
		// response to this call.
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_SET_FLASH:             // 0x0A
		printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		//DUMP_PKT(reqdata);
		// submits the request to the hardware port, don't wait for response
		packet_mux_request(reqdata, &len, false);
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_SET_FLASH_SEQ:             // 0x24
		printf("MFDDRV: Processing %s request...\n", pkt_label[pkt_hdr->pkt_type]);
		//DUMP_PKT(reqdata);
		// the flasher sequence blinking happens in flasher_thread, just save the packet
		// and signal the thread.
		memcpy((char *)&g_flasher_seq_pkt, reqdata, len);
		event_signal(&flasher_event);
		socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));
		break;

	case PACKET_TYPE_I2C_XFER:
		printf("MFDDRV: Processing %s request (%d)...\n", pkt_label[pkt_hdr->pkt_type], datalen);
		// for the EEPROM read/write commands, we need to look at a 
		// sub-command to know if it's a read or write request.
		eeprom_pkt = (eeprom_packet *)reqdata;

		if (eeprom_pkt->i2c_cmd.cmd_type == CMD_TYPE_READ) {

			printf("eeprom_read: write addr phase\n");
			// for reads, we need to first write out the address (which should
			// already be in the first two data bytes) and then read from that
			// address

			// 1. packet already has correct data, just change to write type and size 
			// to 2 (address is two bytes) and write the address

			memcpy(&epkt, reqdata, sizeof(epkt));
			epkt.i2c_cmd.cmd_type = CMD_TYPE_WRITE;
			epkt.i2c_cmd.len = 2;
			len = sizeof(epkt)-30; //64;  //-30
			DUMP_PKT((char *)&epkt);			
			packet_mux_request((char *)&epkt, &len, false);
			usleep(100000);
			//sleep(1);

			// 2. now do the read
			printf("eeprom_read: read data phase\n");
			memcpy(&epkt, reqdata, sizeof(epkt));
			len = sizeof(epkt);	// TODO: only read actual amount
			packet_mux_request((char *)&epkt, &len, true);
			DUMP_PKT((char *)&epkt);
			//usleep(100000);

			// send the response back to the caller (this is a get command)
			socket_if_sendresp((char *)&epkt, len);

		} else if (eeprom_pkt->i2c_cmd.cmd_type == CMD_TYPE_WRITE) {

			printf("eeprom_write (%d)\n", (int)len);
			// for writes, the packet data should already contain
			// the address in the first two data bytes and the len 
			// field should reflect that

			//DUMP_PKT((char *)eeprom_pkt);
			packet_mux_request(reqdata, &len, false);
			usleep(100000);

			// send the response back to the caller (this is a get command)
			socket_if_sendresp((char *)&status_pkt, sizeof(packet_arbiter_status));

		} else {
			// TODO: error handling
			printf("eeprom xfr: invalid operation request!\n");
		}
		break;


	case PACKET_TYPE_GET_DRV_VERSION:
		printf("MFDDRV: Processing GET_DRV_VERSION (%d)\n", (int)len);
		// driver version set locally, don't need to contact hardware
		drv_ver_pkt = (drv_version_packet *)reqdata;
		strcpy(drv_ver_pkt->data, MFDDRV_VERSION);
		// send the response back to the caller (this is a get command)
		socket_if_sendresp(reqdata, min(len, sizeof(drv_version_packet)));
		break;
			
 	default:
 		printf("Received unknown cmd req %x\n", pkt_hdr->pkt_type);
 		break;
    }

    return 1;
}


/*
   Request processing thread - handles API requests which involves a write and a
   read to the hardware.
*/

void* req_thread (void* thread_data)
{
	char reqdata[2048];
	int n = 0, pkt_rem = 0, psize = 0;
	//arbiter_pkt * pkt_hdr;
	char * p;

	printf("req_thread started...(%d)\n", g_halt_requested);

	while (!g_halt_requested) {
	
		// if init fails the first time, it may be due to an improper
		// previous shutdown that didn't free the socket. try a deinit
		// followed by another init attempt.

		if (!socket_if_init()) {
			socket_if_deinit();
			socket_if_init();
		}

		while (!g_halt_requested) {

			// as long as this user-mode driver is running, continue
			// fetching requests. the socket_if_getreq function blocks
			// waiting for data to read from the socket (which represents
			// requests from apps through the API).

		    	printf("Waiting for request...\n");
		        n = socket_if_getreq(reqdata, sizeof(reqdata)); 
		        if (n==0)
		            break;  	// continue?     
		    	//printf("Got request (%d)\n", n);

			// there could be multiple requests in the socket, handle them
			// one at a time.

			pkt_rem = n;				// remaining packet size
			p = reqdata;				// current ptr

			while (pkt_rem >= 2) {
				psize = GET_PKT_SIZE(p);
				//printf("req_thread: n=%d, size=%d\n", n, psize);
				if (pkt_rem >= psize) {
					//printf("REQ_THREAD: pkt_rem =%d, psize=%d\n", pkt_rem, psize);
				        parse_req((char *)p, psize);
					pkt_rem -= psize;
					p += psize;
					//pkt_hdr = (arbiter_pkt *)p;
				//	printf("req_thread2: pkt_rem =%d, psize=%d\n", pkt_rem, psize);
				}
			}						
		}
	}

	printf("req_thread exiting (%d)________________________________________\n", g_halt_requested);
	return NULL;
}


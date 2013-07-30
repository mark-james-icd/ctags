#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <netdb.h> 
#include <sys/un.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

//#define BOARD_PARLAY_DEMO 1
#define SOCKET_IF_PATH  "/IcdMfdSocketPath1"

#include "packet_arbiter.h"
#include "rcu_api.h"


int g_sockfd = 0;
bool g_init = false;

bool client_socket_init();


// the two-byte address has the form:
//   xppppppp ppbbbbbb
//   were x = don't care
//      p = 9-bits for page in eeprom (0-511)
//      b = byte offset in the page

#define EEPROM_PAGE_MASK	0x7FC0		// 01111111 11000000
#define EEPROM_BYTE_MASK	0x003F     	// 00000000 00111111

#define HIBYTE_EEPROM_ADDR(x)	((unsigned char)(((x>>8)&0xFF)))
#define LOBYTE_EEPROM_ADDR(x)	((unsigned char)((x)&0xFF))
#define EEPROM_ADDR(p,b)	((0x00 | ((p << 6) & EEPROM_PAGE_MASK) | (b & EEPROM_BYTE_MASK)))



/* internal functions - not public apis */

bool client_socket_init()
{
	struct sockaddr_un serv_addr;
	struct hostent *server;

	// open the client side of the socket for communication with the mfduart driver
	g_sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (g_sockfd < 0) {
		error("ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_LOCAL;
	strncpy(serv_addr.sun_path, SOCKET_IF_PATH, sizeof(serv_addr.sun_path) - 1);

	if (connect(g_sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		error("ERROR connecting");
	}

	return true;
}

bool rcu_api_init(void)
{
	if (!g_init) {
		client_socket_init();
		g_init = true;
	}

	return true;
}

void rcu_api_deinit(void)
{
	close(g_sockfd);
	g_sockfd = 0;
	g_init = false;
}


/* public api functions */

uint16_t lightring_set_animation(lightring_frame * data, uint8_t numframes)
{
	int n, iled;
	lightring_frame blank_frame;
	lr_frame_packet * lrframe_pkt;
	char databuf[2048];

	rcu_api_init();

	blank_frame.frame_delay = 0;
	for (iled=0; iled < NUM_LR_LEDS; iled++) {
		blank_frame.lr_vals[iled].color      = 0;
		blank_frame.lr_vals[iled].brightness = 0;
	}

	bzero(databuf, 2048);
	lrframe_pkt = (lr_frame_packet *)databuf;
	lrframe_pkt->type = PACKET_TYPE_SET_LRANIMATION;

	if (numframes == 0)
	{
		lrframe_pkt->numframes = 0;
		memcpy((void *)&lrframe_pkt->frames[0], &blank_frame, sizeof(lightring_frame) * numframes);
	}
	else
	{
		//printf("LR Data: %x, %x, %x\n",
		//	data->frame_delay, data->lr_vals[0].color, data->lr_vals[1].color);

		lrframe_pkt->numframes = numframes;
		memcpy((void *)&lrframe_pkt->frames[0], data, sizeof(lightring_frame) * numframes);
	}        

	//printf("PKT: %x, %x, %x %x\n",
	//	lrframe_pkt->type, lrframe_pkt->numframes, lrframe_pkt->frames[0].frame_delay,
	//	lrframe_pkt->frames[0].lr_vals[0].color);

	send_packet(databuf, sizeof(lr_frame_packet) + sizeof(lightring_frame)*(numframes-1));

    return 0;
}

//
// Set lightring animation using RGB values
//

uint16_t lightring_set_animation_rgb(lightring_frame_rgb * data, uint8_t numframes)
{
	int n, iled;
	lightring_frame_rgb blank_frame;
	lr_frame_rgb_packet * lrframe_pkt;
	char databuf[2048];

	rcu_api_init();

	blank_frame.frame_delay = 0;
	for (iled=0; iled < NUM_LR_LEDS; iled++) {
		blank_frame.lr_vals[iled].r = 0;
		blank_frame.lr_vals[iled].g = 0;
		blank_frame.lr_vals[iled].b = 0;
	}

	bzero(databuf, 2048);
	lrframe_pkt = (lr_frame_rgb_packet *)databuf;
	lrframe_pkt->type = PACKET_TYPE_SET_LRANIMATION_RGB;

	if (numframes == 0) {
		lrframe_pkt->numframes = 0;
		memcpy((void *)&lrframe_pkt->frames[0], &blank_frame, sizeof(lightring_frame_rgb) * numframes);
	} else {
		printf("LR Data: %x, %x, %x, %x\n",
			data->frame_delay, data->lr_vals[0].r, data->lr_vals[0].g, data->lr_vals[0].b);

		lrframe_pkt->numframes = numframes;
		memcpy((void *)&lrframe_pkt->frames[0], data, sizeof(lightring_frame_rgb) * numframes);
	}        

	printf("PKT: %x, %x, %x %x\n",
		lrframe_pkt->type, lrframe_pkt->numframes, lrframe_pkt->frames[0].frame_delay,
		lrframe_pkt->frames[0].lr_vals[0].r);

	send_packet(databuf, sizeof(lr_frame_rgb_packet) + sizeof(lightring_frame_rgb)*(numframes-1));

	return 0;
}


//
// Set the lightring to a standard pattern
//

uint16_t lightring_set_animation_pattern(uint8_t pattern_id)
{
	lr_animation_pattern_packet pkt;

	pkt.type = PACKET_TYPE_SET_LRANIMATION_PATTERN;
	pkt.pattern_id = pattern_id;

	send_packet((char *)&pkt, sizeof(lr_animation_pattern_packet));

	return 1;
}


//
// Set lightring brightness scale values.
//

uint16_t lightring_set_brightness_scale(uint8_t brightness_scale)
{
	lr_brightness_packet pkt;

	printf("Setting lightring brightness scale to %d (%d)\n", brightness_scale, (int)sizeof(lr_rgb_packet));

	pkt.type = PACKET_TYPE_SET_BRIGHTNESS;
	pkt.data.brightness = brightness_scale;

	send_packet((char *)&pkt, sizeof(lr_brightness_packet));
	return 1;
}

//
// Set each lightring LED using standard color palette
//

uint16_t lightring_set_colors(lr_packet * pkt)
{
	send_packet((char *)pkt, sizeof(lr_packet));
	return 1;
}

//
// Set each lightring LED using RGB values.
//

uint16_t lightring_set_rgb(lr_rgb_packet * pkt)
{
//	printf("sending packet for set_rgb call (%x, size=%d)\n", pkt->type, (int)sizeof(lr_packet));
	send_packet((char *)pkt, sizeof(lr_rgb_packet));
	return 1;
}


uint16_t sensor_get(uint8_t sensor_id, uint16_t * sensor_value)
{
	char pkt[100];
	temperature_packet * temp_pkt;
	pir_packet * pir_pkt;
	als_packet * als_pkt;
	ir_packet * ir_pkt;
	
	memset(&pkt, 0, sizeof(pkt));

	// format the packet
	switch(sensor_id) {
	case SENSOR_ID_TEMP1:	// MCU temp
		printf("lib: temp1 req: %d\n", 0);
		*sensor_value = 0;
		break;
	case SENSOR_ID_TEMP2:	// RCU temp
		printf("lib: temp2 req: %d\n", (int)sizeof(temperature_packet));
		memset(pkt, 0, sizeof(pkt));
		temp_pkt = (temperature_packet *)pkt;
		temp_pkt->type = PACKET_TYPE_GET_TEMP;
		send_packet((char *)pkt, sizeof(temperature_packet));
		*sensor_value = temp_pkt->value.sensor_value;
		printf("Sensor value: %d\n", *sensor_value);
		break;
	case SENSOR_ID_ALS:	// ambient light sensor
		memset(pkt, 0, sizeof(pkt));
		als_pkt = (als_packet *)pkt;
		als_pkt->type = PACKET_TYPE_GET_ALS;
		send_packet((char *)pkt, sizeof(als_packet));
		*sensor_value = als_pkt->value.sensor_value;
		printf("Sensor value: %d\n", *sensor_value);
		break;
	case SENSOR_ID_PIR:	// proximity sensor	
		memset(pkt, 0, sizeof(pkt));
		pir_pkt = (pir_packet *)pkt;
		pir_pkt->type = PACKET_TYPE_GET_PIR;
		send_packet((char *)pkt, sizeof(pir_packet));
		*sensor_value = pir_pkt->value.sensor_value;
		break;
	case SENSOR_ID_IR:	// raw IR codes
		memset(pkt, 0, sizeof(pkt));
		ir_pkt = (ir_packet *)pkt;
		ir_pkt->type = PACKET_TYPE_GET_IR;
		send_packet((char *)pkt, sizeof(ir_packet));
		*sensor_value = ir_pkt->value.key_code;
		break;
	default:
		break;
	}

	return 1;
}

uint16_t product_id_get(uint8_t * buffer, size_t len)
{
	prodinfo_packet pkt;
	
	if (len < sizeof(pkt))
		printf("product_id_get error: input buffer too small\n");

	memset(&pkt, 0, sizeof(pkt));
	pkt.type = PACKET_TYPE_GET_PRODUCT_ID;
	send_packet((char *)&pkt, sizeof(pkt));
	//printf("PRODUCT_ID: type=%x, %c %c %c %c\n", pkt.type, 
	//	pkt.string[0], pkt.string[1], pkt.string[2], pkt.string[3]);
	memcpy(buffer, pkt.string, sizeof(pkt.string));
	// TODO: temporary fix for bug
	buffer[11]=0;  // not null-terminated
	return 1;
}

uint16_t firmware_version_get(uint8_t * buffer, size_t len)
{
	prodinfo_packet pkt;
	
	if (len < sizeof(pkt))
		printf("firmware_version_get error: input buffer too small\n");

	memset(&pkt, 0, sizeof(pkt));
	pkt.type = PACKET_TYPE_GET_FW_VERSION;
	send_packet((char *)&pkt, sizeof(pkt));
	//printf("FIRMWARE VER: type=%x, %c %c %c %c\n", pkt.type, 
	//	pkt.string[0], pkt.string[1], pkt.string[2], pkt.string[3]);
	
	memcpy(buffer, pkt.string, sizeof(pkt.string));

	return 1;
}

uint16_t serial_number_get(uint32_t * ser_num)
{
	prodinfo_packet pkt;
	
	memset(&pkt, 0, sizeof(pkt));
	pkt.type = PACKET_TYPE_GET_SN;
	send_packet((char *)&pkt, sizeof(pkt));
	//printf("FIRMWARE_VER: type=%x, \n", pkt.type, pkt.value[0]);
	*ser_num = pkt.value[0];

	return 1;
}

uint16_t eeprom_read(char * data, size_t * datalen, uint16_t page, uint8_t byte)
{
	eeprom_packet pkt;
	size_t len;
	uint16_t eeprom_addr = 0;

	// validate parameters
	if (*datalen > EEPROM_MAX_BYTE || page > EEPROM_MAX_PAGE || byte > EEPROM_MAX_BYTE)
		return -1;

	memset(&pkt, 0, sizeof(pkt));

	// fill out an eeprom packet
	pkt.type             = PACKET_TYPE_I2C_XFER;
	pkt.i2c_cmd.bus_no   = 0;
	pkt.i2c_cmd.addr     = 0xA0;
	pkt.i2c_cmd.cmd_type = CMD_TYPE_READ;
	pkt.i2c_cmd.len      = *datalen;

	eeprom_addr = EEPROM_ADDR(page, byte);	
	pkt.i2c_cmd.data[0] = HIBYTE_EEPROM_ADDR(eeprom_addr);
	pkt.i2c_cmd.data[1] = LOBYTE_EEPROM_ADDR(eeprom_addr);

	printf("addr=%x, low=%x, high=%x\n", eeprom_addr, pkt.i2c_cmd.data[0], pkt.i2c_cmd.data[1]);
	send_packet((char *)&pkt, sizeof(pkt));

	if (pkt.i2c_cmd.len < *datalen)
		*datalen = pkt.i2c_cmd.len;

	printf("copying %d bytes to caller buffer\n", (int)*datalen);	
	memcpy(data, pkt.i2c_cmd.data, *datalen);
	return 0;
}

uint16_t eeprom_write(char * data, size_t datalen, uint16_t page, uint8_t byte)
{
	eeprom_packet pkt;
	uint16_t eeprom_addr = 0;
	int i;

	// validate parameters
	if (datalen > EEPROM_MAX_BYTE || page > EEPROM_MAX_PAGE || byte > EEPROM_MAX_BYTE)
		return -1;

	memset(&pkt, 0, sizeof(pkt));

	// fill out an eeprom packet
	pkt.type             = PACKET_TYPE_I2C_XFER;
	pkt.i2c_cmd.bus_no   = 0;
	pkt.i2c_cmd.addr     = 0xA0;
	pkt.i2c_cmd.cmd_type = CMD_TYPE_WRITE;
	pkt.i2c_cmd.len      = datalen+2;

	eeprom_addr = EEPROM_ADDR(page, byte);	
	pkt.i2c_cmd.data[0] = HIBYTE_EEPROM_ADDR(eeprom_addr);
	pkt.i2c_cmd.data[1] = LOBYTE_EEPROM_ADDR(eeprom_addr);

	for (i=2; i < 64; i++)
		pkt.i2c_cmd.data[i] = data[i-2];
	

	//memcpy(&pkt.i2c_cmd.data[2], data, datalen);

//	printf("data: %x %x %x %x %x %x (%d)\n", 
//		data[0], data[1], data[2], data[3], data[4], data[5], pkt.i2c_cmd.len);

//	printf("data: %x %x %x %x %x %x (%d)\n", 
//		pkt.i2c_cmd.data[0], pkt.i2c_cmd.data[1], pkt.i2c_cmd.data[2], 
//		pkt.i2c_cmd.data[3], pkt.i2c_cmd.data[4], pkt.i2c_cmd.data[5], pkt.i2c_cmd.len);

	send_packet((char *)&pkt, sizeof(pkt));
	return 0;
}


uint16_t flasher_set(uint16_t intensity, uint16_t length_on, uint8_t flasher_bitmask)
{
	flasher_packet pkt;

	pkt.type = PACKET_TYPE_SET_FLASH;
	pkt.data.intensity = intensity;
	pkt.data.length    = length_on;
	pkt.data.bank_mask = flasher_bitmask;

	send_packet((char *)&pkt, sizeof(pkt));
	return 0;
}


uint16_t flasher_sequence(uint16_t intensity, uint16_t length_on, uint16_t length_off, 
	uint8_t num_flashes, uint8_t flasher_bitmask)
{
	flasher_seq_packet pkt;

	pkt.type = PACKET_TYPE_SET_FLASH_SEQ;
	pkt.data.intensity   = intensity;
	pkt.data.length_on   = length_on;
	pkt.data.length_off  = length_off;
	pkt.data.flash_count = num_flashes;
	pkt.data.bank_mask = flasher_bitmask;

	send_packet((char *)&pkt, sizeof(pkt));
	return 0;
}

uint16_t set_leds(uint8_t led_id, uint8_t led_intensity)
{
	status_led_packet pkt;

	printf("Setting %d icon LED to %d\n", led_id, led_intensity);

	pkt.type = PACKET_TYPE_SET_LEDS;
	pkt.data.led_id = led_id;
	pkt.data.intensity = led_intensity;

	send_packet((char *)&pkt, sizeof(status_led_packet));
	return 0;
}

uint16_t driver_version_get(char * buffer, size_t len)
{
	drv_version_packet pkt;

	memset(&pkt, 0, sizeof(drv_version_packet));
	pkt.type = PACKET_TYPE_GET_DRV_VERSION;

	send_packet((char *)&pkt, sizeof(drv_version_packet));
	memcpy(buffer, pkt.data, sizeof(pkt.data));
	return 0;
}

// internal functions

int send_packet(char * pkt, int pktsize)
{
	char buffer[256];
	int n;

	// make sure interface to driver is initialized
	rcu_api_init();

//	printf("SENDING PKT: %x %x %x\n", pkt[0], pkt[1], pkt[2]);

	//printf("data: %x %x %x %x %x %x\n", 
	//	pkt[3], pkt[4], pkt[5], pkt[6], pkt[7], pkt[8]);


	n = write(g_sockfd, pkt, pktsize);
	if (n < 0) { 
		printf("ERROR writing to socket");
	}

	//printf("DONE SENDING PKT: %x %x %x\n", pkt[0], pkt[1], pkt[2]);

	bzero(buffer,256);
	n = read(g_sockfd,buffer,255);
	if (n < 0) {
		printf("read error\n");
	}

//	printf("RECEIVED PKT: %x %x %x (%d)\n", buffer[0], buffer[1], buffer[2], n);

// TODO: we get back status packet on many get commands right now, 
// which in a few cases is bigger than the input pkt.

	//memcpy(pkt, buffer, MIN(sizeof(buffer), pktsize));
	if (n > pktsize)
		//printf("ERROR: DATA (%d) TOO LARGE FOR PACKET BUFFER (%d)\n", n, pktsize);
		memcpy(pkt, buffer, pktsize);
	else
		memcpy(pkt, buffer, n);
	return 0;
}

int get_packet(char * pkt, int pktsize)
{
	char buffer[256];
	int n;

	// make sure interface to driver is initialized
	rcu_api_init();

	n = read(g_sockfd, pkt, pktsize);
	if (n < 0) { 
		printf("ERROR reading from socket");
	}

	bzero(buffer,256);
	n = read(g_sockfd,buffer,255);
	if (n < 0) {
		printf("read error\n");
	}

	return 0;
}







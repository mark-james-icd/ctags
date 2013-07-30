#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdbool.h>

#include "lightring_animation.h"

#include "packet_arbiter.h"
#include "rcu_api.h"
#include "event.h"
#include "server.h"
#include "utils.h"

extern bool g_debug;


char pkt_label[0x2F][32] = {
	"STATUS",            	// 0x00
	"SET_LIGHTRING",       	// 0x01
	"SET_LEDS",           	// 0x02
	"GET_TEMP",           	// 0x03
	"GET_IR",              	// 0x04
	"GET_SN",              	// 0x05
	"SET_BRIGHTNESS",      	// 0x06
	"GET_PRODUCT_ID",      	// 0x07
	"SET_LIGHTRING_RGB",   	// 0x08
	"GET_FW_VERSION",      	// 0x09
	"SET_FLASH",           	// 0x0A
	"GET_ALS",             	// 0x0B
	"GET_REMOTE",          	// 0x0C
	"I2C_XFER",            	// 0x0D
	"GET_PIR",             	// 0x0E
	"",			// 0x0F
	"",			// 0x10
	"",			// 0x11
	"",			// 0x12
	"",			// 0x13
	"",			// 0x14
	"",			// 0x15
	"",			// 0x16
	"",			// 0x17
	"",			// 0x18
	"",			// 0x19
	"",			// 0x1A
	"",			// 0x1B
	"",			// 0x1C
	"",			// 0x1D
	"",			// 0x1E
	"",			// 0x1F
	"SET_LRANIMATION",	// 0x20
	"SET_LRANIMATION_RGB",	// 0x21
	"SET_LRANIMATION_PATTERN", // 0x22
	"GET_DRV_VERSION",	// 0x23
	"SET_FLASH_SEQ",	// 0x24
	"",			// 0x25
};

//	"ERROR"               	// 0xFF


size_t pkt_size[0x2F] = {
	0, 	// PACKET_TYPE_STATUS              0x00
	sizeof(lr_packet), 		// PACKET_TYPE_SET_LIGHTRING       0x01
	sizeof(status_led_packet), 	// PACKET_TYPE_SET_LEDS            0x02
	sizeof(temperature_packet), 	// PACKET_TYPE_GET_TEMP            0x03
	sizeof(ir_packet), 		// PACKET_TYPE_GET_IR              0x04
	sizeof(prodinfo_packet),	// PACKET_TYPE_GET_SN              0x05
	sizeof(lr_brightness_packet),	// PACKET_TYPE_SET_BRIGHTNESS      0x06
	sizeof(prodinfo_packet), 	// PACKET_TYPE_GET_PRODUCT_ID      0x07
	sizeof(lr_rgb_packet), 		// PACKET_TYPE_SET_LIGHTRING_RGB   0x08
	sizeof(prodinfo_packet), 	// PACKET_TYPE_GET_FW_VERSION      0x09
	sizeof(flasher_packet), 	// PACKET_TYPE_SET_FLASH           0x0A
	sizeof(als_packet), 		// PACKET_TYPE_GET_ALS             0x0B
	0, 	// PACKET_TYPE_GET_REMOTE          0x0C
	sizeof(eeprom_packet),	 	// PACKET_TYPE_I2C_XFER            0x0D
	sizeof(pir_packet),		// PACKET_TYPE_GET_PIR             0x0E
	0,				//				   0x0F
	0,				//				   0x10
	0,				//				   0x11
	0,				//				   0x12
	0,				//				   0x13
	0,				//				   0x14
	0,				//				   0x15
	0,				//				   0x16
	0,				//				   0x17
	0,				//				   0x18
	0,				//				   0x19
	0,				// 	   			   0x1A
	0,				// 				   0x1B
	0,				// 				   0x1C
	0,				// 				   0x1D
	0,				// 				   0x1E
	0,				// 				   0x1F
	sizeof(lr_frame_packet),	// PACKET_TYPE_SET_LRANIMATION	   0x20
	sizeof(lr_frame_rgb_packet),	// PACKET_TYPE_SET_LRANIMATION_RGB 0x21
	sizeof(lr_animation_pattern_packet),// PACKET_TYPE_SET_LRANIMATION_PATTERN  0x22
	sizeof(drv_version_packet),	//				   0x23
	sizeof(flasher_seq_packet),	//				   0x24
	0,				//				   0x25
	0				//				   0x26
};



size_t GET_PKT_SIZE(char * data)
{
	lr_frame_packet * pkt;
	arbiter_pkt * pkt_hdr;
	size_t size = 0;

	pkt_hdr = (arbiter_pkt *)data;

	if (pkt_hdr->pkt_type == PACKET_TYPE_SET_LRANIMATION) {
		pkt = (lr_frame_packet *)data;
		size = sizeof(lr_frame_packet) + sizeof(lightring_frame)*(pkt->numframes - 1);
		printf("lr_frame_packet size = %d, frames =%d, size = %d\n", (int)sizeof(lr_frame_packet), 
			pkt->numframes, (int)size);

	} else if (pkt_hdr->pkt_type == PACKET_TYPE_SET_LRANIMATION_RGB) {
		pkt = (lr_frame_packet *)data;
		size = sizeof(lr_frame_rgb_packet) + sizeof(lightring_frame_rgb)*(pkt->numframes - 1);
		printf("lr_frame_rgb_packet size = %d, frames =%d, size = %d\n", (int)sizeof(lr_frame_packet), 
			pkt->numframes, (int)size);
	} else {
		size = pkt_size[pkt_hdr->pkt_type];
	}

	return size;		
}

//
// Dump the contents of a packet
//

void DUMP_PKT(char * data)
{
	arbiter_pkt * pkt;
	temperature_packet * temp_pkt;
	als_packet * als_pkt;
	pir_packet * pir_pkt;
 	prodinfo_packet * prodinfo_pkt;
	eeprom_packet * eeprom_pkt;
	flasher_packet * flasher_pkt;
	lr_packet * lr_pkt;
	int i;

	if (data == NULL)
		return;

	if (!g_debug)
		return;

	pkt = (arbiter_pkt *)data;

	switch (pkt->pkt_type) {

	case PACKET_TYPE_STATUS:
	case PACKET_TYPE_ERROR:                 // 0x00
	case PACKET_TYPE_SET_LEDS:              // 0x02	
        	break;

	case PACKET_TYPE_GET_TEMP:              // 0x03
        	temp_pkt = (temperature_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_TEMP (%x), Value=%x\n", 
			temp_pkt->type, temp_pkt->value.sensor_value);
        	break;

	case PACKET_TYPE_GET_ALS:
        	als_pkt = (als_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_ALS (%x), Value=%x\n",
			als_pkt->type, als_pkt->value.sensor_value);
        	break;

	case PACKET_TYPE_GET_PIR:
        	pir_pkt = (pir_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_PIR (%x), Value=%x\n",
			pir_pkt->type, pir_pkt->value.sensor_value);
        	break;

	case PACKET_TYPE_GET_REMOTE:
	case PACKET_TYPE_GET_IR:                // 0x04
        	printf("DUMP_PKT: Type=%x\n", pkt->pkt_type);
		break;

	case PACKET_TYPE_GET_SN:                // 0x05
        	prodinfo_pkt = (prodinfo_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_SN (%x), Value=%x\n", 
			prodinfo_pkt->type, prodinfo_pkt->value[0]);
        	//printf("DUMP_PKT: Type=%x, %x %x %x %x\n", 
		//	prodinfo_pkt->type, 
		//	prodinfo_pkt->string[0], prodinfo_pkt->string[1], 
		//	prodinfo_pkt->string[2], prodinfo_pkt->string[3]);

		printf(" %x %x %x %x %x %x %x %x %x %x %x %x\n",  
			prodinfo_pkt->value[0], prodinfo_pkt->value[1], prodinfo_pkt->value[2],
			prodinfo_pkt->value[3], prodinfo_pkt->value[4], prodinfo_pkt->value[5],
			prodinfo_pkt->value[6], prodinfo_pkt->value[7], prodinfo_pkt->value[8],
			prodinfo_pkt->value[9], prodinfo_pkt->value[10], prodinfo_pkt->value[11]);

		printf(" %x %x %x %x %x %x %x %x %x %x %x %x\n",  
			prodinfo_pkt->string[0], prodinfo_pkt->string[1], prodinfo_pkt->string[2],
			prodinfo_pkt->string[3], prodinfo_pkt->string[4], prodinfo_pkt->string[5],
			prodinfo_pkt->string[6], prodinfo_pkt->string[7], prodinfo_pkt->string[8],
			prodinfo_pkt->string[9], prodinfo_pkt->string[10], prodinfo_pkt->string[11]);
		break;

	case PACKET_TYPE_GET_PRODUCT_ID:        // 0x07
        	prodinfo_pkt = (prodinfo_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_PRODUCT_ID (%x), %c%c%c%c%c%c\n", 
			prodinfo_pkt->type, 
			prodinfo_pkt->string[0], prodinfo_pkt->string[1], prodinfo_pkt->string[2], 
			prodinfo_pkt->string[3], prodinfo_pkt->string[4], prodinfo_pkt->string[5]);
        	break;
 
	case PACKET_TYPE_GET_FW_VERSION:        // 0x09
        	prodinfo_pkt = (prodinfo_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_GET_FW_VERSION (%x), %c%c%c%c%c%c\n",
			prodinfo_pkt->type, 
			prodinfo_pkt->string[0], prodinfo_pkt->string[1], prodinfo_pkt->string[2], 
			prodinfo_pkt->string[3], prodinfo_pkt->string[4], prodinfo_pkt->string[5]);
        	break;
  
	case PACKET_TYPE_SET_BRIGHTNESS:        // 0x06
        	printf("DUMP_PKT: PACKET_TYPE_SET_BRIGHTNESS\n");
		break;
    
	case PACKET_TYPE_SET_LIGHTRING_RGB:     // 0x08
        	printf("DUMP_PKT: PACKET_TYPE_SET_LIGHTRING_RGB\n");
		break;

    	case PACKET_TYPE_SET_FLASH:             // 0x0A
        	//printf("DUMP_PKT: PACKET_TYPE_SET_FLASH (%x)\n", pkt->pkt_type);
		UNUSED(flasher_pkt);
        	//break;
		// pass through for now, i2c is returning A for some reason

	case PACKET_TYPE_I2C_XFER:
        	eeprom_pkt = (eeprom_packet *)data;
       		printf("DUMP_PKT: PACKET_TYPE_I2C_XFER (%x)\n", pkt->pkt_type);
       		printf("DUMP_PKT: bus=%x, addr=%x, cmd_type=%x, len=%x\n", 
			eeprom_pkt->i2c_cmd.bus_no, eeprom_pkt->i2c_cmd.addr,
			eeprom_pkt->i2c_cmd.cmd_type, eeprom_pkt->i2c_cmd.len);
       		//printf("DUMP_PKT: data: %x %x %x %x %x %x %x %x\n", 
		//	eeprom_pkt->i2c_cmd.data[0], eeprom_pkt->i2c_cmd.data[1], 
		//	eeprom_pkt->i2c_cmd.data[2], eeprom_pkt->i2c_cmd.data[3],
		//	eeprom_pkt->i2c_cmd.data[3], eeprom_pkt->i2c_cmd.data[5],
		//	eeprom_pkt->i2c_cmd.data[6], eeprom_pkt->i2c_cmd.data[7]);
			for (i=0; i<32; i++)
				printf("%x ", eeprom_pkt->i2c_cmd.data[i]);
			printf("\n");
        	break;

	case PACKET_TYPE_SET_LIGHTRING:         // 0x01
        	lr_pkt = (lr_packet *)data;
        	printf("DUMP_PKT: PACKET_TYPE_SET_LIGHTRING (%x)\n", pkt->pkt_type);
		printf("LED CLR: ");
		for (i=0; i < NUM_LR_LEDS; i++)
			printf("%d ", lr_pkt->leds[i].color);
		printf("\n");
		printf("LED BRI: ");
		for (i=0; i < NUM_LR_LEDS; i++)
			printf("%d ", lr_pkt->leds[i].brightness);
		printf("\n");

        	break;

	case PACKET_TYPE_SET_LRANIMATION:       // 0x80
        	//buf = (lightring_frame *)data;
        	printf("DUMP_PKT: PACKET_TYPE_SET_LRANIMATION (%x)\n", pkt->pkt_type);
        	break;

	case PACKET_TYPE_SET_LRANIMATION_RGB:       // 0x81
        	//bufrgb = (lightring_frame_rgb *)data;
        	printf("DUMP_PKT: PACKET_TYPE_SET_LRANIMATION_RGB\n");
        	break;

	case PACKET_TYPE_SET_LRANIMATION_PATTERN:       // 0x82
        	//bufrgb = (lightring_frame_rgb *)data;
        	printf("DUMP_PKT: PACKET_TYPE_SET_LRANIMATION_PATTERN\n");
        	break;

	default:
        	printf("Received unknown cmd req %x\n", pkt->pkt_type);
        	break;
	}

}


size_t min(size_t size1, size_t size2)
{
	if (size1 <= size2)
		return size1;
	return size2;
}

void WAIT_PROMPT(void)
{
	char buffer[32];
	char * c = buffer;

	// wait for user prompt
	printf("Press return to continue...\n");
	bzero(buffer, sizeof(buffer));
	c = fgets(buffer, sizeof(buffer) ,stdin);
	UNUSED(c);
	return;
}

/*
void dbg(int level, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
*/


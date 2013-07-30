#ifndef _RCUAPI_H_
#define _RCUAPI_H_

#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "packet_arbiter.h"


#ifndef NUM_LR_LEDS
#if defined(LR16)
#define NUM_LR_LEDS 16
#else
#define NUM_LR_LEDS 8
#endif
#endif

#define ICD_VIRTUAL_INPUT_DEVICE	"icd_uinput"

#define PACKET_TYPE_SET_LRANIMATION		0x20
#define PACKET_TYPE_SET_LRANIMATION_RGB		0x21
#define PACKET_TYPE_SET_LRANIMATION_PATTERN	0x22
#define PACKET_TYPE_GET_DRV_VERSION		0x23
#define PACKET_TYPE_SET_FLASH_SEQ		0x24

#define MAX_ANIMATION_FRAME	32

typedef struct lr_led {
	uint8_t color      : 4;
	uint8_t brightness : 4;
}__attribute__((packed)) lr_led;

typedef struct lr_led_rgb {
	uint8_t r;             // red
	uint8_t g;             // green
	uint8_t b;             // blue
}__attribute__((packed)) lr_led_rgb;

typedef struct lightring_frame {
	int frame_delay;
	lr_led lr_vals[NUM_LR_LEDS];
} __attribute__((packed)) lightring_frame;

typedef struct lightring_frame_rgb {
	int frame_delay;
	lr_led_rgb lr_vals[NUM_LR_LEDS];
} __attribute__((packed)) lightring_frame_rgb;



// high level packet structure definitions (these describes complete packet over the wire, 
// i.e. the header and the data.

typedef struct lr_animation_pattern_packet {
	uint8_t type;
	uint8_t pattern_id;
} __attribute__((packed)) lr_animation_pattern_packet;


typedef struct lr_frame_packet {
	uint8_t type;
	uint8_t numframes;
	lightring_frame frames[1];	// may be more than one frame 
} __attribute__((packed)) lr_frame_packet;

typedef struct lr_frame_rgb_packet {
	uint8_t type;
	uint8_t numframes;
	lightring_frame_rgb frames[1];	// may be more than one frame 
} __attribute__((packed)) lr_frame_rgb_packet;

typedef struct lr_packet {
	uint8_t type;
	lr_led leds[NUM_LR_LEDS];
}__attribute__((packed)) lr_packet;

typedef struct lr_rgb_packet {
	uint8_t type;
	lr_led_rgb leds[NUM_LR_LEDS];
	//struct packet_data_lightring_rgb leds;
}__attribute__((packed)) lr_rgb_packet;

typedef struct lr_brightness_packet {
	uint8_t type;
	struct packet_data_brightness data;
}__attribute__((packed)) lr_brightness_packet;

// send this struct to the mfddrv, which sends a packet_data_flasher struct to the firmware,
// the other fields are used only by mfddrv
struct packet_data_flasher_seq {
	uint16_t intensity; 	/* %DC * 10 */
	uint16_t length_on; 	/* in ms */
	uint16_t length_off; 	/* in ms */
	uint8_t  flash_count; 	/* number of times to blink flashers */
	uint8_t bank_mask;
}__attribute__((__packed__));

typedef struct flasher_seq_packet {
	uint8_t type;
	struct packet_data_flasher_seq data;
}__attribute__((packed)) flasher_seq_packet;

typedef struct flasher_packet {
	uint8_t type;
	struct packet_data_flasher data;
}__attribute__((packed)) flasher_packet;

typedef struct temperature_packet {
	uint8_t type;
	struct packet_data_temperature value;
}__attribute__((packed)) temperature_packet;

typedef struct pir_packet {
	uint8_t type;
	struct packet_data_pir value;
}__attribute__((packed)) pir_packet;

typedef struct als_packet {
	uint8_t type;
	struct packet_data_als value;
}__attribute__((packed)) als_packet;

typedef struct ir_packet {
	uint8_t type;
	struct packet_data_ir value;
}__attribute__((packed)) ir_packet;

typedef struct prodinfo_packet {
	uint8_t type;
	union {
		uint8_t string[48];
		uint32_t value[12];
	};
}__attribute__((packed)) prodinfo_packet;

typedef struct status_led_packet {
	uint8_t type;
	struct packet_data_status_led data;
}__attribute__((packed)) status_led_packet;


/* LIGHTRING APIs */

uint16_t lightring_set_animation(lightring_frame * data, uint8_t numframes);
uint16_t lightring_set_animation_rgb(lightring_frame_rgb * data, uint8_t numframes);

// Request the lightring display a predefined pattern

uint16_t lightring_set_animation_pattern(uint8_t lightring_pattern_id);

#define LIGHTRING_ANIMATION_PATTERN_MIN 0
#define LIGHTRING_SOLID_WHITE		0
#define LIGHTRING_SOLID_RED		1
#define LIGHTRING_SOLID_GREEN		2
#define LIGHTRING_SOLID_BLUE		3
#define LIGHTRING_WRGB_SEQ1		4
#define LIGHTRING_TEST_PATTERN		5
#define LIGHTRING_ANIMATION_PATTERN_MAX 5

// Request the lightring display predefined colors or raw RGB values

#define MAX_LINE_LEN 512
#define MAX_TOKENS 32

uint16_t lightring_set_colors(lr_packet * pkt);

// Set the lightring brightness scale values

uint16_t lightring_set_brightness_scale(uint8_t brightness_scale);

// Set one of the other leds (identified by the led_id values defined above)

uint16_t set_leds(uint8_t led_id, uint8_t led_intensity);

#define SENSOR_ID_TEMP1		0x01 	// for system with only one temp sensor
#define SENSOR_ID_TEMP2		0x02 	// for system with only one temp sensor
#define SENSOR_ID_MCU_TEMP	0x01
#define SENSOR_ID_RCU_TEMP	0x02
#define SENSOR_ID_ALS		0x03
#define SENSOR_ID_PIR		0x04
#define SENSOR_ID_IR		0x05

// Get a current sensor value

uint16_t sensor_get(uint8_t sensor_id, uint16_t * sensor_value);

// Alternatively, the following routines are also available:

uint16_t get_mcu_temp();
uint16_t get_rcu_temp();
uint16_t get_rcu_light();

// Query the serial number (an array of bytes):

uint16_t serial_number_get(uint32_t * ser_num);

// Query the product ID string (an array of characters)

uint16_t product_id_get(uint8_t * buffer, size_t len);

uint16_t firmware_version_get(uint8_t * buffer, size_t len);


// eeprom APIs

#define CMD_TYPE_WRITE 0
#define CMD_TYPE_READ  1

typedef struct i2c_bridge_cmd {
	uint8_t bus_no;
	uint8_t addr;
	uint8_t cmd_type;
	uint8_t len;
	uint8_t data[66]; // 66
}__attribute__((packed)) i2c_bridge_cmd;

typedef struct eeprom_packet {
	uint8_t type;
	struct i2c_bridge_cmd i2c_cmd;
}__attribute__((packed)) eeprom_packet;

typedef struct drv_version_packet {
	uint8_t type;
	char data[64]; // 64
}__attribute__((packed)) drv_version_packet;

#define EEPROM_MAX_PAGE		512
#define EEPROM_MAX_BYTE		64

uint16_t eeprom_read(char * data, size_t * datalen, uint16_t page, uint8_t byte);
uint16_t eeprom_write(char * data, size_t datalen, uint16_t page, uint8_t byte);

uint16_t flasher_set(uint16_t intensity, uint16_t length_on, uint8_t flasher_bitmask);

uint16_t flasher_sequence(uint16_t intensity, uint16_t length_on, uint16_t length_off, 
	uint8_t num_flashes, uint8_t flasher_bitmask);

uint16_t driver_version_get(char * buffer, size_t buffer_len);

#endif /* RCUAPI_H */


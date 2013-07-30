#ifndef _ARBITER_H_
#define _ARBITER_H_

#include <stdint.h>

#define ARBITER_TX_BUF_SIZE 10
#define ARBITER_PKT_DATA_SIZE 50

#ifdef LR16
#define NUM_LR_LEDS 16
#else
#define NUM_LR_LEDS 8
#endif


#define BOARD_NUM_LEDS NUM_LR_LEDS

// packet type values

#define PACKET_TYPE_STATUS              0x00
#define PACKET_TYPE_SET_LIGHTRING       0x01
#define PACKET_TYPE_SET_LEDS            0x02
#define PACKET_TYPE_GET_TEMP            0x03
#define PACKET_TYPE_GET_IR              0x04
#define PACKET_TYPE_GET_SN              0x05
#define PACKET_TYPE_SET_BRIGHTNESS      0x06
#define PACKET_TYPE_GET_PRODUCT_ID      0x07
#define PACKET_TYPE_SET_LIGHTRING_RGB   0x08
#define PACKET_TYPE_GET_FW_VERSION      0x09
#define PACKET_TYPE_SET_FLASH           0x0A
#define PACKET_TYPE_GET_ALS             0x0B
#define PACKET_TYPE_GET_REMOTE          0x0C
#define PACKET_TYPE_I2C_XFER            0x0D
#define PACKET_TYPE_GET_PIR             0x0E
#define PACKET_TYPE_SET_ALS		0x0F
#define PACKET_TYPE_GET_GPIO		0x10
#define PACKET_TYPE_SET_GPIO		0x11
#define PACKET_TYPE_ERROR               0xFF

typedef struct arbiter_pkt {
	uint8_t pkt_type;
	uint8_t data[ARBITER_PKT_DATA_SIZE];
}__attribute__((__packed__)) arbiter_pkt;

/* Status Packets */
typedef struct packet_arbiter_status {
	uint8_t cmd_type;
	int err_code;
}__attribute__((__packed__)) packet_arbiter_status;

/* Palette control scheme */
struct lightring_palette_led {
	uint8_t color_id  : 4;
	uint8_t intensity : 4;
}__attribute__((__packed__));

struct packet_data_lightring_palette {
	struct lightring_palette_led leds[BOARD_NUM_LEDS];    // predefined colors
}__attribute__((__packed__));

/* Full color control scheme */
struct lightring_rgb_led  {
	uint8_t r;             // red
	uint8_t g;             // green
	uint8_t b;             // blue
} __attribute__((__packed__));

struct packet_data_lightring_rgb {
	struct lightring_rgb_led leds[BOARD_NUM_LEDS];
}__attribute__((__packed__));

struct packet_data_brightness {
	uint8_t brightness;
}__attribute__((__packed__));

/* led_color_id values */
#define LED_COLOR_OFF	        0
#define LED_COLOR_ON	        1
#define LED_COLOR_RED		2
#define LED_COLOR_ORANGE	3
#define LED_COLOR_YELLOW	4
#define LED_COLOR_LIME		5
#define LED_COLOR_GREEN		6
#define LED_COLOR_SPRING	7
#define LED_COLOR_CYAN		8
#define LED_COLOR_SKY		9
#define LED_COLOR_BLUE		10
#define LED_COLOR_PURPLE	11
#define LED_COLOR_MAGENTA	12
#define LED_COLOR_PINK		13
#define LED_COLOR_MAX		13
#define LED_COLOR_WHITE	        1

#define LED_BRIGHTNESS_OFF	0x0
#define LED_BRIGHTNESS_LOW	0x1
#define LED_BRIGHTNESS_LOW1	0x2
#define LED_BRIGHTNESS_LOW2	0x3
#define LED_BRIGHTNESS_LOW3	0x4
#define LED_BRIGHTNESS_LOW4	0x5
#define LED_BRIGHTNESS_MEDIUM	0x6
#define LED_BRIGHTNESS_MEDIUM1	0x7
#define LED_BRIGHTNESS_MEDIUM2	0x8
#define LED_BRIGHTNESS_MEDIUM3	0x9
#define LED_BRIGHTNESS_MEDIUM4	0xA
#define LED_BRIGHTNESS_HIGH	0xB
#define LED_BRIGHTNESS_HIGH1	0xC
#define LED_BRIGHTNESS_HIGH2	0xD
#define LED_BRIGHTNESS_HIGH3	0xE
#define LED_BRIGHTNESS_HIGH4	0xF

/* led_id values */
#define LED_ID_RCU_SIGNMAIL     0x01
#define LED_ID_RCU_MISSEDCALL   0x02

struct packet_data_status_led {
	uint8_t led_id;
	uint8_t intensity;                         // PLT - CHANGED THIS!!!
}__attribute__((__packed__));

struct packet_data_flasher {
	uint16_t intensity; /* %DC * 10 */
	uint16_t length; /* in ms */
	uint8_t bank_mask;
}__attribute__((__packed__));

struct packet_data_temperature {
	uint16_t sensor_value;
}__attribute__((__packed__));

struct packet_data_pir {
	uint16_t sensor_value;
}__attribute__((__packed__));

struct packet_data_als {
	uint16_t sensor_value;
}__attribute__((__packed__));

/* sensor_value_type values */
#define SENSOR_VALUE_TYPE_CURRENT_TEMP  0x01
#define SENSOR_VALUE_TYPE_MAX_TEMP      0x02

struct packet_data_ir {
	uint32_t key_code;                  // e.g. 0x0069 => KEY_LEFT
}__attribute__((__packed__));

int arbiter_stage_packet(struct arbiter_pkt *pkt);
void arbiter_tick(void);

#endif /* ARBITER_H */

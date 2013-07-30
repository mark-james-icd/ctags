#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/un.h>
#include <stddef.h>
#include <stdbool.h>

#include "unistd.h"
#include "linux/kd.h"
#include "termios.h"
#include "fcntl.h"
#include "sys/ioctl.h"

#include "packet_arbiter.h"
#include "rcu_api.h"

#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <linux/input.h>
 
#define KEYBOARD_DEV "/dev/input/event3"
#define VIRTUAL_INPUT_DEV "/dev/input/event%d"

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void dump_buff(char * buff, size_t len)
{
	int i;
	for (i=0; i < len; i++)
		printf("%x ", buff[i]);
	printf("\n");
}


bool test_lightring_animation_start(void)
{
	int iframe; 
	int iled;
	lightring_frame lrframes[32];

	/* fill in hard-coded test pattern */

	for (iframe=0; iframe < MAX_ANIMATION_FRAME; iframe++) {	
		lrframes[iframe].frame_delay = 50;
 		for (iled=0; iled < NUM_LR_LEDS; iled++) {
 			lrframes[iframe].lr_vals[iled].color      = iframe+iled+1;
			lrframes[iframe].lr_vals[iled].brightness = LED_BRIGHTNESS_HIGH;
			
 			if (lrframes[iframe].lr_vals[iled].color > LED_COLOR_MAX) {
				lrframes[iframe].lr_vals[iled].color = LED_COLOR_MAX;
			}
		}
	}

	/* start the animation, this will keep going until you stop it */         
	lightring_set_animation(lrframes, 32);

	return true;
}

bool test_lightring_animation_rgb_start()
{
	int iframe; 
	int iled;
	int r=0, g=0, b=0;
	lightring_frame_rgb lrframes[16];

	/* fill in hard-coded test pattern */
	//SETRGB(lrframe[0], 
	bzero(lrframes, sizeof(lrframes));

	// initialize first frame, then shift it
  	lrframes[0].frame_delay = 50;
	iled=0;
	while (iled<NUM_LR_LEDS) {
  		lrframes[0].lr_vals[iled].r = 50; lrframes[0].lr_vals[iled].g =  0; lrframes[0].lr_vals[iled++].r =   0;
  		lrframes[0].lr_vals[iled].r =  0; lrframes[0].lr_vals[iled].g = 50; lrframes[0].lr_vals[iled++].r =   0;
  		lrframes[0].lr_vals[iled].r =  0; lrframes[0].lr_vals[iled].g =  0; lrframes[0].lr_vals[iled++].r =  50;
		continue;
	}

	for (iframe=1; iframe < 16; iframe++) {	
		lrframes[iframe].frame_delay = 50;
 		for (iled=0; iled < NUM_LR_LEDS; iled++) {
			if (iled == NUM_LR_LEDS) {
	 			lrframes[iframe].lr_vals[iled].r = lrframes[iframe-1].lr_vals[0].r;
				lrframes[iframe].lr_vals[iled].g = lrframes[iframe-1].lr_vals[0].g;
				lrframes[iframe].lr_vals[iled].b = lrframes[iframe-1].lr_vals[0].b;
			} else {
	 			lrframes[iframe].lr_vals[iled].r = lrframes[iframe-1].lr_vals[iled+1].r;
				lrframes[iframe].lr_vals[iled].g = lrframes[iframe-1].lr_vals[iled+1].g;
				lrframes[iframe].lr_vals[iled].b = lrframes[iframe-1].lr_vals[iled+1].b;
			}
		}
	}

	/* start the animation, this will keep going until you stop it */         
	lightring_set_animation_rgb(lrframes, 16);

	return true;
}

bool test_lightring_animation_pattern_start()
{
	int i;

	printf("Cycle through each predefined lightring animation pattern..\n");

	// test each predefined lightring pattern (let each for 5 secs)
	for (i=LIGHTRING_ANIMATION_PATTERN_MIN; i <= LIGHTRING_ANIMATION_PATTERN_MAX; i++)
	{
		lightring_set_animation_pattern(i);
		sleep(5);
		lightring_set_animation(NULL, 0);
		sleep(1);
	}

	return true;
}

bool test_lightring_animation_stop(void)
{
	/* clear the lightring, also cancels any animation in progress */
	lightring_set_animation(NULL, 0);
	lightring_set_animation_rgb(NULL, 0);
	return true;
}

bool test_lightring_set_solid(int solid_color)
{
	lr_packet lr_pkt;
	int i = 0;

	// send lightring to specified solid color (for all LEDs)
	lr_led solidclr = {
		.color = solid_color,
		.brightness = 6,
	};

	lr_pkt.type = PACKET_TYPE_SET_LIGHTRING;

	for (i = 0; i < NUM_LR_LEDS; i++) {
		lr_pkt.leds[i].color      = solidclr.color;
		lr_pkt.leds[i].brightness = solidclr.brightness;
	}

	lightring_set_colors(&lr_pkt);
	return true;
}

bool test_lightring_set_rgb(int r, int g, int b)
{
	lr_rgb_packet pkt;
	int i = 0;

	printf("Setting LEDs to RGB: %d %d %d (size=%d)\n", r, g, b, (int)sizeof(lr_rgb_packet));

	pkt.type = PACKET_TYPE_SET_LIGHTRING_RGB;

	for (i = 0; i < NUM_LR_LEDS; i++) {
		pkt.leds[i].r = r;
		pkt.leds[i].g = g;
		pkt.leds[i].b = b;
	}

	lightring_set_rgb(&pkt);
	return true;
}


bool test_lightring_set_brightness_scale(int brightness_scale)
{
	lightring_set_brightness_scale(brightness_scale);
	return true;
}

bool test_sensor_get()
{
	uint16_t value;

	// get each sensor value

	// TEMP2 - on board the LR board
	sensor_get(SENSOR_ID_TEMP2, &value);
	printf("TEMP 2: %d\n", value);

	// ALS
	sensor_get(SENSOR_ID_ALS, &value);
	printf("ALS:    %d\n", value);

	return true;
}

bool test_get_product_info()
{
	char buffer[64];
	uint32_t ser_num = 0;

	bzero(buffer, sizeof(buffer));
	product_id_get(buffer, sizeof(buffer));
	printf("Product ID:    %s\n", buffer);

	memset(buffer, 0, sizeof(buffer));
	bzero(buffer, sizeof(buffer));
	firmware_version_get(buffer, sizeof(buffer));
	printf("Firmware Ver:  %s\n", buffer);

	memset(buffer, 0, sizeof(buffer));
	serial_number_get(&ser_num);
	printf("Serial Number: %x %d\n", ser_num, ser_num);

	memset(buffer, 0, sizeof(buffer));
	driver_version_get(buffer, sizeof(buffer));
	printf("Driver version: %s\n", buffer);
}


bool test_ir()
{
 	int input_fd = 0;
 	struct input_event keyboard_ev;
 	char name[256];
	char input_name[256];
	int bytes;
	int i;
	int exit_count = 0;

	// find the virtual input device
	for (i=0; i < 10 && input_fd<=0; i++) {
		sprintf(input_name, VIRTUAL_INPUT_DEV, i);
		input_fd = open(input_name, O_RDONLY | O_NONBLOCK);
		if (input_fd > 0) {
			if (ioctl(input_fd, EVIOCGNAME(256), name) < 0) {
				printf("EVICGNAME ioctl called failed\n");
				close(input_fd);
				input_fd = 0;
			} else {
				if (strcmp(ICD_VIRTUAL_INPUT_DEVICE, name)) {
					// keep looking
					printf("not this one: %s\n", input_name);
					close(input_fd);
					input_fd = 0;
                        	} else {
					printf("Found virtual device: %s, %s\n", input_name, name);
				}
			}
		}
	}

	if (input_fd <= 0) {
		printf("Couldn't find virtual input device\n");
		return false;
	}

	printf("This test captures IR codes stuffed in the virtual uinput device.\n"); 
	printf("Use a text editor to verify IR codes stuffed in the keyboard input device.\n");
	printf("Hit 'enter' three times in a row to exit this test...\n");

	while (true) {
		bytes = read(input_fd, &keyboard_ev, sizeof(keyboard_ev));
		if (bytes > 0) {
			if (keyboard_ev.type & EV_KEY && keyboard_ev.value == 0) {
				if (keyboard_ev.code == 0x10)
					break;

				switch (keyboard_ev.code) {
				case KEY_UP:
					printf("\nKEY_UP\n");
					exit_count = 0;
					break;
				case KEY_DOWN:
					printf("\nKEY_DOWN\n");
					exit_count = 0;
					break;
				case KEY_LEFT:
					printf("\nKEY_LEFT\n");
					exit_count = 0;
					break;
				case KEY_RIGHT:
					printf("\nKEY_RIGHT\n");
					exit_count = 0;
					break;
				case KEY_ENTER:
					printf("\nKEY_ENTER\n");
					exit_count++;
				}

				if (exit_count >= 3)
					break;
			}
		}

		usleep(1000);
	}

        close(input_fd);
	return true;
}

bool test_pir(void)
{
	int i;
	uint16_t value;

	// PIR
	printf("This tests polls the PIR for 20 seconds (500 msec intervals)...\n");
	for (i = 0; i < 40; i++) {
		sensor_get(SENSOR_ID_PIR, &value);
		printf("PIR: %d\n", value);
		usleep(10000);
	}
	return true;
}

bool test_eeprom(void)
{
	char wbuff[64];
	char rbuff[64];
	int i;
	size_t len;

	// write/read to first page
	memset(rbuff, 0, sizeof(rbuff));
	memset(wbuff, 0, sizeof(wbuff));
	for (i = 0; i < sizeof(wbuff); i++)
		wbuff[i] = 5;

	printf("Writing data to eeprom:\n");	
	eeprom_write(wbuff, sizeof(wbuff), 0, 0);

	printf("Reading date from eeprom:\n");
	len = sizeof(rbuff);
	eeprom_read(rbuff, &len, 0, 0);

	for (i = 0; i < 64; i++)
		printf("%x ", rbuff[i]);
	printf("\n");

	sleep(1);
/*
	// write/read 12th page, 8th byte
	memset(rbuff, 0, 32);
	memset(wbuff, 0, 32);
	for (i = 0; i < 64; i++)
		wbuff[i] = i;

	eeprom_write(wbuff, sizeof(wbuff), 12, 8);
	len = sizeof(rbuff);
	eeprom_read(rbuff, &len, 12, 8);

	for (i = 0; i < 32; i++)
		printf("%x ", rbuff[i]);
	printf("\n");
*/

	return true;
}


bool test_set_leds()
{	
	// turn on the signmail icon LED for 3 seconds, then turn off
	set_leds(LED_ID_RCU_SIGNMAIL, 80);
	sleep(3);
	set_leds(LED_ID_RCU_SIGNMAIL, 0);

	// turn on the missedcall icon LED for 3 seconds, then turn off
	set_leds(LED_ID_RCU_MISSEDCALL, 80);
	sleep(3);
	set_leds(LED_ID_RCU_MISSEDCALL, 0);

	return true;
}

bool test_flashers()
{
	// flash the flashers manually

	// intensity (%DC*10), length_on (ms), length_off (ms), bank_mask
	flasher_set(100, 300, 0xFF);
	usleep(300000);
	flasher_set(100, 300, 0xFF);
	usleep(300000);
	flasher_set(100, 300, 0xFF);

	sleep(1);

	// request a sequence: full bright, 5 flashes, 20ms on, 300 ms off, all flashers

	flasher_sequence(1000, 20, 300, 5, 0xFF);

	return true;
}


void test_display_menu(void)
{
	printf("Enter command:\n");
	printf("   a. Start a lightring animation (color ids)\n");
	printf("   b. Start a lightring animation (rgb values)\n");
	printf("   c. Start a lightring animation (cycle through predefined patterns)\n");
	printf("   d. Clear the light ring (stops any animation in progress)\n");
	printf("   e. Set lightring pattern to solid lime (color id)\n");
	printf("   f. Set lightring pattern to solid purple (rgb value)\n");
        printf("   g. Set lightring brightness scale (10)\n");
	printf("   h. Get Sensor Values\n");
	printf("   i. Test IR\n");
        printf("   j. Test PIR\n");
	printf("   k. Get serial number and product id\n");
	printf("   l. Test EEPROM read and writes\n");
        printf("   m. Set icon LEDs\n");
	printf("   n. Test Flashers\n");
	printf("   q. quit\n");
}


int main(int argc, char *argv[])
{
    char buffer[256];

    printf("ClientTest (LR%d)\n", NUM_LR_LEDS);

    while (true) {
	// display test menu
	test_display_menu();
	
	// get test selection
        bzero(buffer,256);
        fgets(buffer,255,stdin);

        if (buffer[0] == 'q')
            break;

 	switch (buffer[0]) {
 	case 'a':
		// start light ring animation using colors ids
		printf("Starting light ring animation test (color ids)...\n");
		test_lightring_animation_start();
		break;
 	case 'b':
		// start light ring animation using rgb values
		printf("Starting light ring animation test (rgb values)...\n");
		test_lightring_animation_rgb_start();
		break;
 	case 'c':
		// start light ring animation using predefined pattern
		printf("Starting light ring animation test (predefined patterns)...\n");
		test_lightring_animation_pattern_start();
		break;
	case 'd':
		// stop light ring animation
		printf("Stopping light ring animation test...\n");
		test_lightring_animation_stop();
		break;
	case 'e':
		// set lightring to lime (pallet)
		printf("Setting lightring to lime (color id)...\n");
		test_lightring_set_solid(LED_COLOR_LIME);
                break;
	case 'f':
		// set lightring to purle (rgb)
		printf("Setting lightring to purple (rgb value)...\n");
		test_lightring_set_rgb(102, 0, 204);
                break;
	case 'g':
		// set lightring brightness scale
		printf("Setting lightring brightness scale (10)...\n");
		test_lightring_set_brightness_scale(40);
                break;
	case 'h': 
		// read sensor values
		printf("Get sensor values...\n");
		test_sensor_get();
		break;
	case 'i': 
		// read sensor values
		printf("Test IR...\n");
		test_ir();
		break;
	case 'j': 
		// read sensor values
		printf("Test PIR...\n");
		test_pir();
		break;
	case 'k': 
		// get serial number and product id
		printf("Get serial number and product id...\n");
		test_get_product_info();
		break;
	case 'l': 
		// get serial number and product id
		printf("Test eeprom read and writes...\n");
		test_eeprom();
		break;
	case 'm':
		// set the icon LEDs
		printf("Test the icon LEDs...\n");
		test_set_leds();
		break;
	case 'n':
		// test flashers
		printf("Test the flashers...\n");
		test_flashers();
		break;
	default:
		printf("unknown cmd %d\n", buffer[0]);
		break;
        }
    }
    
    printf("Shutting down\n");
    return 0;
}


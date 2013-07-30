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

bool set_led_rgb(int led, int r, int g, int b)
{
    lr_rgb_packet pkt = {0};
    pkt.type = PACKET_TYPE_SET_LIGHTRING_RGB;
    pkt.leds[led].r = r;
    pkt.leds[led].g = g;
    pkt.leds[led].b = b;
    lightring_set_rgb(&pkt);
}

bool test_lightring()
{
    int i = 0;
    for (i = 0; i < NUM_LR_LEDS; i++) {
        set_led_rgb(i,255,0,0);
        usleep(250*1000);
        set_led_rgb(i,0,255,0);
        usleep(250*1000);
        set_led_rgb(i,0,0,255);
        usleep(250*1000);
        set_led_rgb(i,255,255,255);
        usleep(250*1000);
    }

    lightring_set_animation_rgb(NULL, 0);
}

bool test_lightring_set_brightness_scale(int brightness_scale)
{
	lightring_set_brightness_scale(brightness_scale);
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


bool test_set_leds()
{	
    lightring_set_brightness_scale(LED_BRIGHTNESS_HIGH4);

    // turn on the signmail icon LED for 3 seconds, then dims it, then turn off
    lightring_set_brightness_scale(LED_BRIGHTNESS_HIGH4);
    set_leds(LED_ID_RCU_SIGNMAIL, 80);
    sleep(3);
    lightring_set_brightness_scale(LED_BRIGHTNESS_LOW);
    set_leds(LED_ID_RCU_SIGNMAIL, 80);
    sleep(3);
    set_leds(LED_ID_RCU_SIGNMAIL, 0);

    // turn on the missedcall icon LED for 3 seconds, then dims it, then turn off
    lightring_set_brightness_scale(LED_BRIGHTNESS_HIGH4);
    set_leds(LED_ID_RCU_MISSEDCALL, 80);
    sleep(3);
    lightring_set_brightness_scale(LED_BRIGHTNESS_LOW);
    set_leds(LED_ID_RCU_MISSEDCALL, 80);
    sleep(3);
    set_leds(LED_ID_RCU_MISSEDCALL, 0);

	return true;
}

bool test_flashers()
{	    
    flasher_sequence(1000, 20, 300, 3, 0xFF);
}

int main(int argc, char *argv[])
{

    if (argc < 2) {
        printf("Invalid arguments, missing test number to run :-\n");
        printf("1 - test flashers\n");
        printf("2 - test indicators\n");
        printf("3 - test lightright\n");
        printf("4 - display product info\n");
        exit(0);
    }

    int test = atoi(argv[1]);
    int time = 0; // seconds

    time = 10;
    if (argc > 2) {
        time = atoi(argv[2]);
    }

    printf("Lightring test (LR%d)\n", NUM_LR_LEDS);

    switch (test) {
    case 1:
        printf("testing flashers\n");
        test_flashers();
		break;
    case 2:
        printf("testing indicators\n");
        test_set_leds();
		break;
    case 3:
        printf("testing lightright\n");
        lightring_set_brightness_scale(LED_BRIGHTNESS_HIGH4);
        test_lightring();
        lightring_set_brightness_scale(LED_BRIGHTNESS_LOW);
        test_lightring();
		break;
    case 4:
        test_get_product_info();
        break;
    }

    printf("Shutting down\n");
    return 0;
}


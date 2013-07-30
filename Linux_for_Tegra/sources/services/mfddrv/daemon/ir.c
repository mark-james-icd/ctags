// linux include files
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h> 

// include files for this program
#include "packet_arbiter.h"
#include "rcu_api.h"
#include "event.h"
#include "packet_mux.h"
#include "utils.h"
#include "ir.h"

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

 
#define KEYBOARD_DEV "/dev/input/event8"
#define VIRTUAL_INPUT1 "/dev/uinput"
#define VIRTUAL_INPUT2 "/dev/uinput/uinput"

#define UINPUT_MAX_NAME_SIZE    80

extern bool g_halt_requested;
extern pthread_mutex_t response_mutex;
extern char resp_buffer[2048];
extern arbiter_pkt * g_response_pkt;
extern size_t        g_response_pkt_len;
extern custom_event response_event;
int g_keyboard_fd = 0;
int g_virtual_input_fd = 0;

int create_virtual_input_device(void)
{
	struct uinput_user_dev uidev;
	int ret;
	int fd;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		printf("Failed opening uinput (%d)\n", fd);
		return -1;
	}

	// enable key press, release events
	ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
	if (ret < 0) {
		printf("ioctl UI_SET_EVBIT - EV_KEY failed (%d)", ret);
		return -1;
	}

	// enable key synchronization events
	ret = ioctl(fd, UI_SET_EVBIT, EV_SYN);
	if (ret < 0) {
		printf("ioctl UI_SET_EVBIT - EV_SYN failed (%d)", ret);
		return -1;
	}

	// enable which keycodes are allowed to be sent via the input subsystem
	ret = ioctl(fd, UI_SET_KEYBIT, KEY_UP);
	ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
	ret = ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);
	ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
	ret = ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
	if (ret < 0) {
		printf("ioctl UI_SET_KEYBIT failed (%d)\n", ret);
		return -1;
	}

	// fill out uinput_user_dev struct and write it to file descriptor
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "icd_uinput");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1234;
	uidev.id.product = 0xfedd;
	uidev.id.version = 1;

	ret = write(fd, &uidev, sizeof(uidev));
	if (ret < 0) {
		printf("Sending uidev struct to uinput failed (%d)\n", ret);
		return -1;
	}
	
	// create the device
	ret = ioctl(fd, UI_DEV_CREATE);
	if (ret < 0) {
		printf("UI_DEV_CREATE failed (%d)\n", ret);
		return -1;
	}

	// fd is now file descriptor of the new virtual input device
	g_virtual_input_fd = fd;
	return 0;
}


bool is_keybd(uint32_t bitmask)
{
	// Both a mouse and keyboard will have the EV_SYN and EV_KEY bit set 
	// (they both need synchronization frames and both have keys). Only mice
	// will have EV_ABS and EV_REL (pointer) while only keyboard will have 
	// EV_REP.

	// keyboards support EV_SYN
	if (!(bitmask & (1 << EV_SYN))) {
		return false;
	}
 
	// keyboards support EV_KEY
	if (!(bitmask & (1 << EV_KEY))) {
		return false;
	}
 
	// keyboards don't support EV_REL
	if ((bitmask & (1 << EV_REL))) {
		return false;
	}
 
	// keyboards don't support EV_ABS 
	if ((bitmask & (1 << EV_ABS))) {
		return false;
	}

	// keyboards support EV_KREP
	if (!(bitmask & (1 << EV_REP))) {
		return false;
	}
 
	return true;
}

bool ir_init()
{
 	char name[256];
	char input_name[256];
	int i = 0;
	int fd = 0;
	uint32_t bitmask = 0;

	g_keyboard_fd = 0;

	// create a virtual input device to sent IR events to in case a keyboard
	// isn't available

	create_virtual_input_device();

	// find the keyboard device programmatically

	for (i = 0; i < 10 && g_keyboard_fd <= 0; i++) {

		sprintf(input_name, "/dev/input/event%d", i);
		//printf("Testing %s\n", input_name);

		fd = open(input_name, O_RDWR | O_NONBLOCK);	// O_SYNC
		if (fd > 0) {
			memset(name, 0, sizeof(name));
			bitmask = 0;

			// get the name
			ioctl(fd, EVIOCGNAME(256), name);

			// get the device's supported input event types
			ioctl(fd, EVIOCGBIT(0, EV_MAX), &bitmask);

			//printf("Device Name (%d): %s, %x\n", i, name, bitmask);
			// check if it's a keyboard device
			if (bitmask > 0 && is_keybd(bitmask)) {
				g_keyboard_fd = fd;
				printf("Keyboard device found: %s on %s\n", name, input_name);
			} else {
				//printf("Not a keyboard device\n");
				close(fd);
			}

		} else {
			//printf("Failed opening keyboard device %s\n", input_name);
		}
	}

	if (g_keyboard_fd <= 0)
		printf("No keyboard device found\n");

	return true;
}

void ir_deinit()
{
	// destropy the virtual input device we created
	if (g_virtual_input_fd != 0) {
		ioctl(g_virtual_input_fd, UI_DEV_DESTROY);
		close(g_virtual_input_fd);
		g_virtual_input_fd = 0;
	}

	// close the keyboard input device if opened
	if (g_keyboard_fd != 0) {
	        close(g_keyboard_fd);
		g_keyboard_fd = 0;
	}
}

uint32_t translate_code(uint32_t keycode)
{
	uint32_t converted_code = 0;

	switch (keycode) {
	case 0x10:	// up-arrow
		converted_code = KEY_UP;	// 0x67
		break;
	case 0x11:	// left-arrow
		converted_code = KEY_LEFT;	// 0x69
		break;
	case 0x12:	// OK/enter
		converted_code = KEY_ENTER;	// 0x1c
		break;
	case 0x13:	// right-arrow
		converted_code = KEY_RIGHT;	// 0x6a
		break;
	case 0x14:	// down-arrow
		converted_code = KEY_DOWN;	// 0x6c
		break;
	case 0x81:
		// this a repeat key I think, don't map for now
		converted_code = 0x0;
		break;		
	default:
		printf("unmapped keycode (%x)\n", keycode);
		converted_code = 0x0;
		break;
	}

	return converted_code;
}


bool stuff_keycode(uint32_t keycode, int input_fd)
{
	struct input_event key;
	int bytes;

	if (input_fd <= 0)
		return false;

	// to stuff the event into the keyboard input queue, send a key pressed event, 
	// then a key released event, and then a sync event to terminate the complete
	// event

	key.type  = EV_KEY;
	key.value = 1;	// pressed
	key.code  = (uint16_t)keycode;

	bytes = write(input_fd, &key, sizeof(key));
	if (bytes <= 0)
		printf("keybrd write failed, bytes=%x\n", (unsigned int)bytes);

	key.type  = EV_KEY;
	key.value = 0;	// released
	key.code  = (uint16_t)keycode;

	bytes = write(input_fd, &key, sizeof(key));
	if (bytes <= 0)
		printf("keybrd write failed, Res=%x\n", (unsigned int)bytes);

	key.type = EV_SYN;
	key.value = 0;	// N/A
	key.code = 0;	// N/A

	bytes = write(input_fd, &key, sizeof(key));
	if (bytes <= 0)
		printf("keybrd write failed, Res=%x\n", (unsigned int)bytes);

	return true;
}

bool process_keycode(uint32_t keycode)
{
	struct input_event key;
	int bytes;
	uint32_t converted_code;

	converted_code = translate_code(keycode);
	if (converted_code == 0x0)
		return true;	// ignore

	printf("IR CODE: code=%x (%x)\n", converted_code, keycode);

	// stuff the keycode in both the keyboard input and the virtual uinput
	stuff_keycode(converted_code, g_keyboard_fd);
	stuff_keycode(converted_code, g_virtual_input_fd);

	return true;
}


//
// This thread handles all read requests which includes polling for IR events and
// processing responses to requests.
//

void* ir_thread (void* thread_data)
{
	uint16_t ret;
	uint8_t buffer[200];
	size_t buffer_len;
	arbiter_pkt * arb_pkt;
	ir_packet * ir_pkt;

	printf("ir_thread starting...\n");

	ir_init();

	arb_pkt = (arbiter_pkt *)buffer;
	ir_pkt = (ir_packet *)buffer;

	while (!g_halt_requested) {

		// check for any data ready to be received
		buffer_len = sizeof(buffer);
		ret = packet_mux_receive((char *)buffer, &buffer_len);
		if (ret <= 0) {
			//printf("ir_thread: ret 0\n");
		} else if (arb_pkt->pkt_type == 0x0) {
			printf("ir_thread: pkt_type 0 (ignore) (%d)\n", (int)buffer_len);
			event_signal(&response_event);
		} else if (arb_pkt->pkt_type < 0x01 || arb_pkt->pkt_type > 0x0E) {
			printf("ir_thread: invalid pkt type %x\n", arb_pkt->pkt_type);
		} else if (arb_pkt->pkt_type == PACKET_TYPE_GET_IR) {
			// process IR codes directly
			process_keycode(ir_pkt->value.key_code);
			printf("RESPONSE: type=%x,  %x (%d)\n", 
				arb_pkt->pkt_type, ir_pkt->value.key_code, (int)buffer_len);
		} else {
			// lock the response_mutex to guard the response packet data
			pthread_mutex_lock(&response_mutex);
			//printf("ir_thread: locked response_mutex\n");
			g_response_pkt_len = min(buffer_len, sizeof(resp_buffer));
			memcpy(g_response_pkt, buffer, g_response_pkt_len);
			printf("RESPONSE: type=%x,  %x %x %x %x (%d)\n", 
				arb_pkt->pkt_type, buffer[1], buffer[2], buffer[3], buffer[4],  (int)buffer_len);

			// unlock the response_mutex so the caller can access the data
			pthread_mutex_unlock(&response_mutex);
			//printf("ir_thread: unlocked response_mutex\n");

			// signal that a response packet is available
			event_signal(&response_event);
		}

	        usleep(RESPONSE_WAIT);
	}

	ir_deinit();
	printf("ir_thread exiting_________________________________________\n");
	return NULL;
}


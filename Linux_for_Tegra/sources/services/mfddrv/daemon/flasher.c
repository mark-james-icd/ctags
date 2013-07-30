// linux include files
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <pthread.h> 

// include files for this program
#include "packet_arbiter.h"
#include "rcu_api.h"
#include "event.h"
#include "packet_mux.h"
#include "utils.h"
#include "flasher.h"

extern bool g_halt_requested;
extern flasher_seq_packet g_flasher_seq_pkt;


void flasher_clear()
{
	// TODO: the flashers only stay on for short time, limited by firmware, this
	// make not be necessary.
}

bool flasher_init()
{
	memset(&g_flasher_seq_pkt, 0, sizeof(flasher_seq_packet));
	return true;
}

bool flasher_deinit()
{
	flasher_clear();
	return true;
}


//
// This thread handles flasher sequence requests.
//

void* flasher_thread (void* thread_data)
{
	custom_event * flasher_event;
	flasher_packet pkt;
	size_t len;
	int count;

	printf("starting flasher_thread...\n");

	flasher_init();

	// the thread data contains a pointer to a custom manual-reset event
	// for syncronizing flasher sequences.
	flasher_event = (custom_event *)thread_data;

	while (!g_halt_requested) {

		event_wait(flasher_event);

		if (g_halt_requested)
			break;

		if (g_flasher_seq_pkt.data.flash_count > FLASHER_COUNT_MAX)
			break;

		// fill out a flasher_packet to send to the RCU firmware
		pkt.type = PACKET_TYPE_SET_FLASH;
		pkt.data.intensity = g_flasher_seq_pkt.data.intensity;
		pkt.data.length    = g_flasher_seq_pkt.data.length_on;
		pkt.data.bank_mask = g_flasher_seq_pkt.data.bank_mask;

		// TODO: for now, since we will likely never flash for long, don't worry about
		// being able break out of this once started, just go ahead and finish it out.

		for (count=0; count < g_flasher_seq_pkt.data.flash_count; count++) {

			// submit the request
			len = sizeof(flasher_packet);
			packet_mux_request((char *)&pkt, &len, false);

			// give the packet time to propagate
			usleep(2500);
			// Wait for the specified delay time between flashes
			usleep(g_flasher_seq_pkt.data.length_off * 1000);
		}
		
		g_flasher_seq_pkt.data.flash_count = 0;
		event_reset(flasher_event);
	}

	// thread is exiting, clear any running animations.
	flasher_deinit();

	printf("Exiting flasher_thread\n");
	return NULL;
}


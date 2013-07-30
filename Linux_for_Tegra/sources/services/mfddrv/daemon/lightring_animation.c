//#define BOARD_PARLAY_DEMO   1


#include <stdio.h>
#include <stdbool.h>

#include <packet_arbiter.h>
#include <rcu_api.h>
#include <lightring_animation.h>
#include <packet_mux.h>
#include <event.h>
#include <utils.h>


extern bool g_halt_lr_requested;

bool         g_debug1;
unsigned int g_cur_frm;
bool         g_animation_rgb = false;
unsigned int g_animation_frames = 0;
lightring_frame g_frames[MAX_ANIMATION_FRAME];
lightring_frame_rgb g_frames_rgb[MAX_ANIMATION_FRAME];


// solid animation patterns (by definition they have only one frame that blinks

#if (NUM_LR_LEDS==16)
lightring_frame g_solid_white = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#else
lightring_frame g_solid_white = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#endif

#if (NUM_LR_LEDS==16)
lightring_frame g_solid_red = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#else
lightring_frame g_solid_red = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#endif


#if (NUM_LR_LEDS==16)
lightring_frame g_solid_green = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#else
lightring_frame g_solid_green = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#endif

#if (NUM_LR_LEDS==16)
lightring_frame g_solid_blue = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#else
lightring_frame g_solid_blue = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#endif

#if (NUM_LR_LEDS==16)
lightring_frame g_wrgb_seq1 = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 }
	}
};
#else
lightring_frame g_wrgb_seq1 = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_WHITE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 },
		{ .color = LED_COLOR_OFF, .brightness = 0 }
	}
};
#endif

#if (NUM_LR_LEDS==16)
lightring_frame g_test_pattern_1 = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_ORANGE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_YELLOW, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_LIME, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_SPRING, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_CYAN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_SKY, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_PURPLE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_MAGENTA, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_PINK, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_ON, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_OFF, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_RED, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#else
lightring_frame g_test_pattern_1 = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = LED_COLOR_LIME, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_GREEN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_SPRING, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_CYAN, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_SKY, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_BLUE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_PURPLE, .brightness = LED_BRIGHTNESS_MEDIUM4 },
		{ .color = LED_COLOR_MAGENTA, .brightness = LED_BRIGHTNESS_MEDIUM4 }
	}
};
#endif

#if (NUM_LR_LEDS==16)
lightring_frame g_solid_off = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 }
	}
};
#else
lightring_frame g_solid_off = {
	  .frame_delay = 100, 
          .lr_vals = {
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 },
		{ .color = 0, .brightness = 0 }
	}
};
#endif


bool lightring_animator_init(bool debug)
{
	g_cur_frm = 0;
	g_debug1 = debug;
	g_animation_rgb = false;
	return true;
}

void lightring_animator_deinit()
{
	return;
}


//
// Save the animation frames so we can play animation after the call returns.
//

int lightring_animator_parse(char * data, int bufsize, bool * animate)
{
	lr_frame_packet * pkt;
	lr_frame_rgb_packet * pkt_rgb;
	arbiter_pkt     * pkt_hdr;

	// validate parameters
	// if (bufsize !=0 && bufsize <= (int)sizeof(lr_frame_packet))

	g_cur_frm = 0;
	pkt = (lr_frame_packet *)data;

	// TODO: change rcu_api.lib to set numframes to zero and then fill out blank
	// frame here or define a new clear packet.

	if (data == NULL || bufsize == 0 || pkt->numframes <= 1)
	{
		printf("Clearing lightring (%d)...\n", bufsize);
		*animate = false;
		// if null pointer passed in or buffer too small, 
		// stop the animation.
		lightring_animator_clear();
	}
	else
	{
		*animate = true;
		pkt_hdr = (arbiter_pkt *)data;

		if (pkt_hdr->pkt_type == PACKET_TYPE_SET_LRANIMATION) {

			printf("Starting lightring animation COLORS (%d)...\n", bufsize);
			g_animation_rgb = false;
			pkt = (lr_frame_packet *)data;
			g_animation_frames = pkt->numframes;

			printf("PKT: %x, %x, %x, %x\n",
				pkt->type, pkt->numframes, 
				pkt->frames[0].frame_delay, pkt->frames[1].frame_delay);

			// save the animation frames
			bzero(g_frames, sizeof(g_frames));
			memcpy((void *)g_frames, (void *)&pkt->frames[0], 
				sizeof(lightring_frame) * pkt->numframes);

			printf("PKT: %x\n", g_frames[0].frame_delay);
			usleep(g_frames[g_cur_frm].frame_delay*1000);

		} else if (pkt_hdr->pkt_type == PACKET_TYPE_SET_LRANIMATION_RGB) {

			printf("Starting lightring animation RGB (%d)...\n", bufsize);
			g_animation_rgb = true;
			pkt_rgb = (lr_frame_rgb_packet *)data;
			g_animation_frames = pkt_rgb->numframes;

			printf("PKT: %x, %x, %x, %x\n",
				pkt_rgb->type, pkt_rgb->numframes, 
				pkt_rgb->frames[0].frame_delay, pkt_rgb->frames[1].frame_delay);

			// save the animation frames
		        bzero(g_frames_rgb, sizeof(g_frames_rgb));
			memcpy((void *)g_frames_rgb, (void *)&pkt_rgb->frames[0], 
				sizeof(lightring_frame_rgb) * pkt_rgb->numframes);

			printf("PKT: %x\n", g_frames_rgb[0].frame_delay);
		        usleep(g_frames_rgb[g_cur_frm].frame_delay*1000);

		} else {
			return 0;
		}
	}

	return 0;
}


uint16_t lightring_animator_parse_pattern(uint8_t pattern_id, bool * animate)
{
	*animate = false;
 	g_cur_frm = 0;
	g_animation_rgb = false;	// patterns always use palette colors
	//lightring_frame frame;
	int iframe, iled;

	printf("Starting lightring animation PATTERN (%d)...\n", pattern_id);
 	bzero(g_frames, sizeof(g_frames));

	switch (pattern_id) {
	case LIGHTRING_SOLID_WHITE:
		printf("WHITE\n");
		// flashing white - consists of a white and off frame
 		g_animation_frames = 2;
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_solid_white, 
	 		sizeof(lightring_frame) * g_animation_frames);
	 	memcpy((void *)&g_frames[1], 
	 		(void *)&g_solid_off, 
	 		sizeof(lightring_frame) * g_animation_frames);

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	case LIGHTRING_SOLID_RED:
		printf("RED\n");
		// flashing red - consists of a red and off frame
 		g_animation_frames = 2;
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_solid_red, 
	 		sizeof(lightring_frame) * g_animation_frames);
	 	memcpy((void *)&g_frames[1], 
	 		(void *)&g_solid_off, 
	 		sizeof(lightring_frame) * g_animation_frames);

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	case LIGHTRING_SOLID_GREEN:
		printf("GREEN\n");
		// flashing red - consists of a green and off frame
 		g_animation_frames = 2;
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_solid_green, 
	 		sizeof(lightring_frame) * g_animation_frames);
	 	memcpy((void *)&g_frames[1], 
	 		(void *)&g_solid_off, 
	 		sizeof(lightring_frame) * g_animation_frames);

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	case LIGHTRING_SOLID_BLUE:
		printf("BLUE\n");
		// flashing red - consists of a blue and off frame
 		g_animation_frames = 2;
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_solid_blue, 
	 		sizeof(lightring_frame) * g_animation_frames);
	 	memcpy((void *)&g_frames[1], 
	 		(void *)&g_solid_off, 
	 		sizeof(lightring_frame) * g_animation_frames);

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	case LIGHTRING_WRGB_SEQ1:
		printf("WRGB_SEQ1\n");
		// flashing red - consists of white, red, green, blue chasing tail sequence
 		g_animation_frames = 16;
		// copy the initial (predefined) frame
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_wrgb_seq1, 
	 		sizeof(lightring_frame) * g_animation_frames);

		// copy the other frames shifted over by one
		for (iframe=1; iframe < 16; iframe++) {	
			g_frames[iframe].frame_delay = g_frames[iframe-1].frame_delay;
			g_frames[iframe].lr_vals[0].color = g_frames[iframe-1].lr_vals[NUM_LR_LEDS-1].color;
			g_frames[iframe].lr_vals[0].brightness = g_frames[iframe-1].lr_vals[NUM_LR_LEDS-1].brightness;
			for (iled=1; iled < NUM_LR_LEDS; iled++) {
				g_frames[iframe].lr_vals[iled].color = g_frames[iframe-1].lr_vals[iled-1].color;
				g_frames[iframe].lr_vals[iled].brightness = g_frames[iframe-1].lr_vals[iled-1].brightness;
			}
		}

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	case LIGHTRING_TEST_PATTERN:		// todo - place holder for test pattern
		// flashing test pattern - consists of a test pattern 1 and off frame
		printf("TEST_PATTERN\n");
 		g_animation_frames = 2;
	 	memcpy((void *)&g_frames[0], 
	 		(void *)&g_test_pattern_1, 
	 		sizeof(lightring_frame) * g_animation_frames);
	 	memcpy((void *)&g_frames[1], 
	 		(void *)&g_solid_off, 
	 		sizeof(lightring_frame) * g_animation_frames);

		printf("PKT: %x %x\n", g_frames[0].frame_delay, g_frames[1].frame_delay);
 		usleep(g_frames[g_cur_frm].frame_delay*1000);
		break;

	default:
		printf("default\n");
		break;
	}

	*animate = true;
	return 0;
}

void lightring_animator_clear(void)
{
	lr_packet lr_pkt;
	size_t len = sizeof(lr_packet);
	int i;

	lr_led blank = {
		.color = 0,
		.brightness = 0,
	};

	printf("Clearing lightring\n");

	// it doesn't matter whether the current animation is based
	// on solid colors or rgb values, just clear it by sending
	// a request with all LEDs off (color id pkt is smaller so
	// using that one, but either would work).

	lr_pkt.type = PACKET_TYPE_SET_LIGHTRING;

	for (i = 0; i < NUM_LR_LEDS; i++) {
		lr_pkt.leds[i].color      = blank.color;
		lr_pkt.leds[i].brightness = blank.brightness;
	}

	// submit the request
	packet_mux_request((char *)&lr_pkt, &len, false);

	/* give the packet time to propegate */
	usleep(2500);
}

//
// this thread handles right ring animations
//

void* lightring_animation_thread (void* thread_data)
{
	lr_packet lr_pkt;
	lr_rgb_packet lr_rgb_pkt;
	custom_event * lr_event;
	size_t len;

	// the thread data contains a pointer to a custom manual-reset event
	// for syncronizing lightring animations.
	lr_event = (custom_event *)thread_data;

	printf("starting lightring_animation_thread...\n");

	while (!g_halt_lr_requested) {

		event_wait(lr_event);

		if (g_halt_lr_requested)
			break;

		// increment the animation frame (wrap if we're already
		// at the last frame).
		g_cur_frm++;
		if (g_cur_frm == g_animation_frames)
			g_cur_frm = 0;

		// the packet to send to the hardware varies depending
		// on whether RGB values or color palette IDs are used.

		if (g_animation_rgb) {

			// prepare an RGB packet to display the current frame

			// printf("rgb=%x, frames=%d, delay=%d\n", g_animation_rgb, 
			//	g_animation_frames, g_frames_rgb[g_cur_frm].frame_delay);

			lr_rgb_pkt.type = PACKET_TYPE_SET_LIGHTRING_RGB;
			memcpy(&lr_rgb_pkt.leds, g_frames_rgb[g_cur_frm].lr_vals, sizeof(lr_rgb_pkt.leds));

			// submit the request
			len = sizeof(lr_rgb_packet);
			packet_mux_request((char *)&lr_rgb_pkt, &len, false);

			// give the packet time to propegate
			usleep(2500);
			// Wait for the specified delay time between frames
			usleep(g_frames_rgb[g_cur_frm].frame_delay*1000);
		} else {

			// prepare a color ID packet to display the current frame

			printf("lr_thread: rgb=%x, frames=%d, delay=%d\n", g_animation_rgb, 
			 	g_animation_frames, g_frames[g_cur_frm].frame_delay);

			lr_pkt.type = PACKET_TYPE_SET_LIGHTRING;
			memcpy(lr_pkt.leds, g_frames[g_cur_frm].lr_vals, sizeof(lr_pkt.leds));

			// submit the request
			len = sizeof(lr_packet);
			packet_mux_request((char *)&lr_pkt, &len, false);

			// give the packet time to propegate
			usleep(2500);
			// Wait for the specified delay time between frames
			usleep(g_frames[g_cur_frm].frame_delay*1000);
		}
	}

	// thread is exiting, clear any running animations.
	lightring_animator_clear();

	printf("Exiting lightring_animation_thread________________________\n");
	return NULL;
}




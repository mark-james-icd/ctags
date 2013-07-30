#ifndef _LIGHTRING_ANIMATION_H_
#define _LIGHTRING_ANIMATION_H_

//#include <packet_mux.h>
//#include <vector>
#include <time.h>

//#include <fstream>
//#include <cstring>
//#include <unordered_map>


// remove this!!!!
/*
typede/f struct lr_led_rgb {
	uint8_t r;             // red
	uint8_t g;             // green
	uint8_t b;             // blue
}__attribute__((packed)) lr_led_rgb;

typedef struct lightring_frame_rgb {
	int frame_delay;
	lr_led_rgb lr_vals[16];
} __attribute__((packed)) lightring_frame_rgb;
typedef struct lr_frame_rgb_packet {
	uint8_t type;
	uint8_t numframes;
	lightring_frame_rgb frames[1];	// may be more than one frame 
} __attribute__((packed)) lr_frame_rgb_packet;
*/


#define MAX_LINE_LEN 512
#define MAX_TOKENS 32

/*
struct lightring_frame {
	int frame_delay;
	lr_led lr_vals[16];
};
*/

void* lightring_animation_thread (void* thread_data);
void* req_thread (void* thread_data);

bool lightring_animator_init(bool debug);
void lightring_animator_deinit();
int lightring_animator_parse(char * data, int bufsize, bool * animate);
//int lightring_animator_parse_rgb(lightring_frame_rgb * data, int bufsize, bool * animate);
void lightring_animator_run(void);
void lightring_animator_clear(void);
int lightring_animator_next_frame(void);
int lightring_animator_get_frame_period(void);
int lightring_animator_get_frame_vals(void);


uint16_t lightring_animator_parse_pattern(uint8_t pattern_id, bool * animate);


/*
class lightring_animator {
public:
	lightring_animator(packet_mux *mux, bool debug);
	int parse_script(char *script_name);
	void run(void);
	void clear_lightring(void);

private:
	int _next_frame(void);
	int get_frame_period(std::ifstream *fin, lightring_frame *frm);
	int get_frame_vals(std::ifstream *fin, lightring_frame *frm);
	packet_mux *mux;
	unsigned int _cur_frm;
	bool _debug;
	std::vector<lightring_frame> frames;

	static const std::unordered_map<std::string, uint8_t> color_map;	
	static const std::unordered_map<std::string, uint8_t> intensity_map;
};
*/

#endif /* LIGHTRING_ANIMATION_H */

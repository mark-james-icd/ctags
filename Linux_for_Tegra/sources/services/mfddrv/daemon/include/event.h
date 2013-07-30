#ifndef _RCUEVENT_H_
#define _RCUEVENT_H_

#include <stdio.h>
#include <pthread.h> 


typedef struct custom_event {
	bool            event_signaled;
    bool            event_initialized;
	int             event_signal_counter;
    pthread_cond_t  event_condition;
    pthread_mutex_t event_mutex;
} custom_event;

// functio prototypes for interacting with custom events

bool event_init(custom_event * evt);
bool event_deinit(custom_event * evt);
bool event_signal(custom_event * evt);
bool event_reset(custom_event * evt);
bool event_wait(custom_event * evt);


#endif /* RCUEVENT_H */

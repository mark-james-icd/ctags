#include <stdbool.h>
#include <stdio.h>
#include <pthread.h> 
#include <errno.h>
#include <time.h>
#include "event.h"


//
// Initialize the custom synchronization event (like a Windows manual-reset style event)
///

bool event_init(custom_event * evt)
{
    // validate input parameters
    if (evt == NULL)
        return false;

    // if someone accidently calls init twice, don't return an error
    if (!evt->event_initialized) {
        // creating a manual-reset type event (like Windows), requires a
        // mutex and a condition.
        pthread_mutex_init(&evt->event_mutex, NULL);
        pthread_cond_init(&evt->event_condition, NULL);
        evt->event_initialized = true;
    }

    return true;
}

//
// Free resources for the custom event
//

bool event_deinit(custom_event * evt)
{
    // validate input parameters
    if (evt == NULL)
        return false;

    // if someone accidently calls deinit twice, don't return an error
    if (evt->event_initialized) {
       	pthread_mutex_destroy(&evt->event_mutex);
        pthread_cond_destroy(&evt->event_condition);
        evt->event_initialized = false;
    }

    return true;
}

//
// Wait on the passed in custom event (if already signalled, wait will return
// immediately - like a windows manual-reset type event)
//

bool event_wait(custom_event * evt)
{
    // validate input parameters
    if (evt == NULL)
        return false;

    // verify event data has been initialized
    if (!evt->event_initialized)
        return false;

    // wait for the event to be signaled -- will return if already signaled
    if (evt->event_initialized) {
        pthread_mutex_lock(&evt->event_mutex);
        int signal_value = evt->event_signal_counter;
        while (!evt->event_signaled && signal_value == evt->event_signal_counter)
        {
            pthread_cond_wait(&evt->event_condition, &evt->event_mutex);
        }
        pthread_mutex_unlock(&evt->event_mutex);
    }

    return true;
}


//
// This performs a zero time wait which effectively checks whether the event is
// signaled or not without waiting.
//
int event_timed_wait(custom_event * evt)
{
    int err = 0;
    struct timespec ts;

    // validate input parameters
    if (evt == NULL)
        return EINVAL;

    // verify event data has been initialized
    if (!evt->event_initialized)
        return EINVAL;

    // using current time is the same as zero relative time wait
//    clock_gettime(CLOCK_REALTIME, &ts); 

    // wait for the event to be signaled -- will return if already signaled
    if (evt->event_initialized) {
        pthread_mutex_lock(&evt->event_mutex);
        int signal_value = evt->event_signal_counter;
        while (!evt->event_signaled && signal_value == evt->event_signal_counter)
        {
            err = pthread_cond_timedwait(&evt->event_condition, &evt->event_mutex, &ts);
        }
        pthread_mutex_unlock(&evt->event_mutex);
    }

    return err;
}


//
// Signal the event (anyone waiting will be released). Event stays signalled
// until manually reset.
//

bool event_signal(custom_event * evt)
{
    // validate input parameters
    if (evt == NULL)
        return false;

    // verify event data has been initialized
    if (!evt->event_initialized)
        return false;

    // signal the event
    //printf("Event signaled\n");
    pthread_mutex_lock(&evt->event_mutex);
    evt->event_signaled = true;
    evt->event_signal_counter++;
    pthread_cond_broadcast(&evt->event_condition);
    pthread_mutex_unlock(&evt->event_mutex);

    return true;
}

//
// Reset the custom event (it will no longer be signalled).
//

bool event_reset(custom_event * evt)
{
    // validate input parameters
    if (evt == NULL)
        return false;

    // verify event data has been initialized
    if (!evt->event_initialized)
        return false;

    // reset the event
    //printf("Event reset\n");
    pthread_mutex_lock(&evt->event_mutex);
    evt->event_signaled = false;
    pthread_mutex_unlock(&evt->event_mutex);

    return true;
}


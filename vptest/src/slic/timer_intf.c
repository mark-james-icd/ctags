/*
** $Id: dummy_timer.c 3411 2012-04-04 23:11:51Z nizajerk $
**
** system.c
** System specific functions implementation file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the system specific functions like timer functions.
**
** Dependancies:
** datatypes.h
**
*/
#include "si_voice_datatypes.h"
#include "si_voice_timer_intf.h"
#include "timer_intf.h"

#include <time.h>
#include <stdio.h>
//#include <stdlib.h>

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj){
	struct timespec t;
    //__time_t tv_sec;		/* Seconds.  */
    //long int tv_nsec;		/* Nanoseconds.  */
    clock_getres(CLOCK_MONOTONIC_RAW, &t);
	printf("TimerInit clock_getres %ld\n", t.tv_nsec);
}


/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs){
	printf("time_DelayWrapper %d\n", timeInMs);
	nanosleep((struct timespec[]){{0, 1000000L  * timeInMs}}, NULL);
	return 0;
}

static struct timespec start;


/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs){
	struct timespec end;
	printf("main\n");

	clock_gettime(CLOCK_MONOTONIC_RAW, &end);

	*timeInMs = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
	printf("time_DelayWrapper %d\n", *timeInMs);

	return 0;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, void *time){
	printf("time_GetTimeWrapper\n");
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    // BUG time is not a returned pointer

    return 0;
}

/*
** $Log: dummy_timer.c,v $
** Revision 1.3  2008/03/13 18:40:03  lajordan
** fixed for si3226
**
** Revision 1.2  2007/10/22 21:38:31  lajordan
** fixed some warnings
**
** Revision 1.1  2007/10/22 20:49:21  lajordan
** no message
**
**
*/


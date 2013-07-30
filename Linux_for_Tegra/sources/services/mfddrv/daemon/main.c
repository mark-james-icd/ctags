/* Copyright (c) 2013 iCD
   This is the entrypoint for the mfddrv multifunction user-mode driver.
   This driver supports a number of devices for a custom hardware platform
   that are all exposed of a single hardware communications channel.
*/

/* linux include files */
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h> 
#include <sysexits.h>

/* custom include files */
#include "packet_arbiter.h"
#include "rcu_api.h"
#include "event.h"
#include "ir.h"
#include "server.h"
#include <serial_port.h>
#include <packet_mux.h>
#include <lightring_animation.h>
#include <flasher.h>
#include "utils.h"

/* global variables */
custom_event lr_event;
custom_event req_event;
custom_event ir_event;
custom_event running_event;
custom_event response_event;
custom_event flasher_event;

pthread_mutex_t ser_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t request_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t response_mutex = PTHREAD_MUTEX_INITIALIZER;

bool g_halt_requested = false;
bool g_halt_lr_requested = false;
volatile bool g_ctrl_c = false;
bool g_debug = true;
bool g_app_mode = false;

/* 
   This function handles the shutodnw signal.
*/

void sigint_handler(int s)
{
	printf("\nClosing...\n");
	//g_halt_requested = true;
	g_ctrl_c = true;
}


/*
   Initialize the MFD driver and any subcomponents.
*/

bool driver_init(void)
{
	if (!packet_mux_init(g_debug))
		return false;

	if (!lightring_animator_init(g_debug)) {
		packet_mux_deinit();
		return false;
	}

	return true;
}


/*
   Uninitialize the MFD driver and any subcomponents.
*/

void driver_deinit()
{
	// unitialize synchronization objects
	event_deinit(&running_event);
	event_deinit(&lr_event);
	event_deinit(&req_event);
	event_deinit(&ir_event);
	event_deinit(&response_event);
	event_deinit(&flasher_event);
	
	// unitialize the socket interface so no more commands are accepted
	socket_if_deinit();

	// make sure the lightring LEDs are off
	//lightring_animator_clear();

	// unitialize subcomponents
	lightring_animator_deinit();
	packet_mux_deinit();

	// unlink the socket
	unlink(SOCKET_IF_PATH);

	return;
}

/*
    Run the process as a daemon
*/
bool run_as_daemon()
{
	pid_t process_id = 0;
	pid_t sid = 0;
	int res;

	// create a child process
	process_id = fork();		
	if (process_id < 0) {
		printf("Creating child process failed\n");
		return false;
	}

	// kill the parent process
	if (process_id > 0) {
		printf("Killing parent process\n");
		exit(0);	// return success
	}

	// the child process is orphaned now, taken over by init process.
	// run new process in a new session and new group. At that point
	// the process is a daemon process without a controlling terminal.
	umask(0);	// unmask the file mode
	sid = setsid();	// set new sessoin
	if(sid < 0) {
		exit(1);	// return failure
	}

	// change the working directory of the daemon process to root, close stdin,
	// stdout, stderr.
	res = chdir("/");	// change current working directory to root
	UNUSED(res);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return true;
}

int main(int argc, char **argv)
{
	struct sigaction sigIntHandler;
	pthread_t req_thread_id;
	pthread_t lr_thread_id;
	pthread_t ir_thread_id;
	pthread_t flasher_thread_id;
	int i;

	printf("g_halt_requested=%d\n", g_halt_requested);

	// check if debug mode enabled on start
	if (0 < argc) {
		for (i=0; i < argc; i++) {
			if (strcmp(argv[i], "-d") == 0) {
				g_debug = true;
				printf("Enabling debug mode\n");
			} else if (strcmp(argv[i], "-a") == 0) {
				g_app_mode = true;
				printf("Run process as a normal application\n");
			}
		}
	}

// TODO: hard code for now!!
g_app_mode = true;

	if (!g_app_mode) {
		if (!run_as_daemon()) {
			printf("Failed switching to daemon, running as application\n");
		}
	}

	// initialize the driver, including hardware communication channel and socket.
	if (!driver_init())
		return -1;
	
	// create a manual reset event that is signalled until we're ready to shutdown.
	event_init(&running_event);
	event_signal(&running_event);

	// create manual reset events for synchronizing request, IR, and animation threads
	event_init(&lr_event);
	event_init(&req_event);
	event_init(&ir_event);
	event_init(&response_event);

 	// create lightring animation, request processing, input processing and flasher thread
	pthread_create(&lr_thread_id, NULL, &lightring_animation_thread, &lr_event);
	pthread_create(&req_thread_id, NULL, &req_thread, &req_event);
	pthread_create(&ir_thread_id, NULL, &ir_thread, &ir_event);
	pthread_create(&flasher_thread_id, NULL, &flasher_thread, &flasher_event);
  
	// handle catching of Ctrl-c in order to shut off the light ring
	sigIntHandler.sa_handler = sigint_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	printf("Waiting for ctrl-c...\n");

	// TODO: replace gobal boolean with an event
	while(!g_ctrl_c) {
		sleep(3);
	}

	// stage the shutdown process...
	// signal the lightring animation thread first so that if an anmation 
	// is in progress we can exit gracefully from the animation thread and
	// turn-off the lightring LEDS

	printf("ctrl-c received\n");
	g_halt_lr_requested = true;
	event_signal(&lr_event); 
	event_signal(&flasher_event);
	sleep(1);
	event_signal(&ir_event); 
	g_halt_requested = true;
	event_signal(&response_event); 
	event_signal(&req_event); 

	// unitialize the driver and subcomponents
	driver_deinit();

	printf("Exiting driver main thread\n");
	return 0;
}


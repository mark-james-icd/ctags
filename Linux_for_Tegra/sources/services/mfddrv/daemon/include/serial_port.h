////////////////////////////////////////////////////////////
// serial-port.cpp: Asynchronous serial port class	  //
// (c) 2012 ICD						  //
// Jeff Ciesielski <jeff.ciesielski@convergeddevices.net> //
////////////////////////////////////////////////////////////
#ifndef __SERIAL_PORT_H___
#define __SERIAL_PORT_H___
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>


void serial_port_init(char *devpath, uint32_t baud);
void serial_port_deinit(void);
int serial_port_open(void);
void serial_port_set_rts(bool on);
void serial_port_set_dtr(bool on);
int serial_port_puts(char *s);
int serial_port_putchar(char c);
int serial_port_write(char *buf, int len);
int serial_port_read(char *s, int len);
int serial_port_getchar(char *c);
void serial_port_close(void);

//struct termios oldtio, newtio;

/*
class serial_port {
public:
	serial_port(char *devpath, uint32_t baud);
	~serial_port(void);
	int open(void);
	void set_rts(bool on);
	void set_dtr(bool on);
	int puts(char *s);
	int putchar(char c);
	int write(char *buf, int len);
	int read(char *s, int len);
	int getchar(char *c);
	void close(void);
private:	
	int fd;
	struct termios oldtio, newtio;
	char devpath[128];
	uint32_t baud;
	enum {
		SERIAL_PORT_STATE_UNKNOWN,
		SERIAL_PORT_STATE_FAILED,
		SERIAL_PORT_STATE_INIT,
		SERIAL_PORT_STATE_OPEN,
	} state;

};
*/

#endif /* __SERIAL_PORT_H___ */

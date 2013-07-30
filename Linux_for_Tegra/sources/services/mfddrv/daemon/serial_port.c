////////////////////////////////////////////////////////////
// serial-port.cpp: Asynchronous serial port class	  //
// (c) 2012 ICD						  //
// Jeff Ciesielski <jeff.ciesielski@convergeddevices.net> //
////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <serial_port.h>
#include <termios.h>
#include <linux/ioctl.h>
#include <linux/tty.h>

char g_devpath[128] = {0};
uint32_t g_baud;
int g_fd;
FILE * g_fp;
struct termios oldtio, newtio;

enum statexxx {
	SERIAL_PORT_STATE_UNKNOWN,
	SERIAL_PORT_STATE_FAILED,
	SERIAL_PORT_STATE_INIT,
	SERIAL_PORT_STATE_OPEN,
};

enum statexxx g_state;



void serial_port_init(char *devpath, uint32_t baud)
{
	if (devpath)
		strcpy(g_devpath, devpath);
		
	g_baud = baud;
}

int serial_port_open(void)
{
	int ret = 0;
	int ldisc = 0;
	
	if (*g_devpath == 0) {
		ret = -ENOENT;
		goto ERR;
	}
	
	// TODO: hardcoded to /dev/ttyUSB0 for now
	g_fd = open(g_devpath, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (g_fd < 0) {
		printf("serial_port_open of %s returned %d\n",g_devpath, g_fd);
		ret = -EBADF;
		goto ERR;
	}


	// by default, the line discipline attached to the serial port is N_TTY, 
	// change it to MFD_LD.

	ldisc = 25; // TODO: N_MFD_LD; fix include file issue, defined in tty.h
	ret = ioctl(g_fd, TIOCSETD, &ldisc);
	if (ret < 0)
		printf("failed setting line discipline to MFD_LD\n");

	sleep(1);

	tcgetattr(g_fd, &oldtio);
	bzero(&newtio, sizeof(newtio));

	/* BAUDRATE: set the bps rate for the this
	 * CS8: set 8n1 (8-bit, no parity, 1 stop bit)
	 * CLOCAL: this is a local connection, no modem control
	 * CREAD: enable receiving
	 */
	newtio.c_cflag = g_baud | CS8 | CLOCAL | CREAD;

	/* IGNPAR: ignore bytes with parity errors
	 * ICRNL: map \r to \n
	 */
	newtio.c_iflag |= IGNPAR | ICRNL;

	/* raw output */
	newtio.c_oflag = 0;

	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;

	/* flush the this and activate the new settings */
	tcflush(g_fd, TCIFLUSH);
	tcsetattr(g_fd, TCSANOW, &newtio);

	g_state = SERIAL_PORT_STATE_OPEN;

	return 0;

ERR:
	g_state = SERIAL_PORT_STATE_FAILED;
	return ret;
}

void serial_port_set_rts(bool on)
{
	int controlbits;

	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		ioctl(g_fd, TIOCMGET, &controlbits);
		if (on) {
			controlbits |= TIOCM_RTS;
		} else {
			controlbits &= ~TIOCM_RTS;
		}
		ioctl(g_fd, TIOCMSET, &controlbits);
	}
}

void serial_port_set_dtr(bool on)
{
	int controlbits;

	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		ioctl(g_fd, TIOCMGET, &controlbits);
		if (on) {
			controlbits |= TIOCM_DTR;
		} else {
			controlbits &= ~TIOCM_DTR;
		}
		ioctl(g_fd, TIOCMSET, &controlbits);
	}
}

int serial_port_puts(char *s)
{
	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		return write(g_fd, s, strlen(s));
	}

	return -ENODEV;
}

int serial_port_putchar(char c)
{
	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		return write(g_fd, &c, 1);
	}

	return -ENODEV;
}

int serial_port_write(char *buf, int len)
{
	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		return write(g_fd, buf, len);
	}

	return -ENODEV;
}

int serial_port_getchar(char *c)
{
	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		return read(g_fd, c, 1);
	}

	return -ENODEV;
}

int serial_port_read(char *s, int len)
{
	
	if (g_fd >= 0 && g_state >= SERIAL_PORT_STATE_OPEN) {
		return read(g_fd, s, len);
	}

	return -ENODEV;
}

void serial_port_close(void)
{
	if (g_fd >= 0) {
		/* restore the old this settings */
		tcsetattr(g_fd, TCSANOW, &oldtio);
		close(g_fd);
		g_fd = -1;
	}

	g_state = SERIAL_PORT_STATE_INIT;
	
}

void serial_port_deinit(void)
{
	serial_port_close();
}


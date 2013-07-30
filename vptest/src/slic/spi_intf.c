#include "si_voice_datatypes.h"
#include "proslic.h"
#include "spi_intf.h"

#include "slic.h"

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>


static int spi_fd = -1;
static const char *device = "/dev/spidev0.0";
static uint8_t mode = SPI_CPHA | SPI_CPOL;
static uint8_t bits = 8;
static uint32_t speed = 200000;
static uint16_t delay = 0;

static int ser_fd = -1;
static const char *serdevice = "/dev/ttyHS3";
static struct termios oldterminfo;

int openserial(const char *devicename)
{
    int fd;
    struct termios attr;

    if ((fd = open(devicename, O_RDWR | O_NDELAY)) == -1) {
        perror("openserial(): open()");
        return 0;
    }

    if (tcgetattr(fd, &oldterminfo) == -1) {
        perror("openserial(): tcgetattr()");
        return 0;
    }
	printf("c_cflag %0x\n", oldterminfo.c_cflag);
	printf("c_oflag %0x\n", oldterminfo.c_oflag);

    attr = oldterminfo;
    attr.c_cflag |= CRTSCTS | CLOCAL;
    attr.c_oflag = 0;
//    if (tcflush(fd, TCIOFLUSH) == -1) {
//        perror("openserial(): tcflush()");
//        return 0;
//    }
    if (tcsetattr(fd, TCSANOW, &attr) == -1) {
        perror("initserial(): tcsetattr()");
        return 0;
    }

    return fd;
}

void closeserial(int fd)
{
    tcsetattr(fd, TCSANOW, &oldterminfo);
    if (close(fd) < 0)
        perror("closeserial()");
}

int setRTS(int fd, int level)
{
    int status;

    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("setRTS(): TIOCMGET");
        return 0;
    }
    if (level)
        status |= TIOCM_RTS;
    else
        status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("setRTS(): TIOCMSET");
        return 0;
    }
    return 1;
}

uInt8 SpiReadByte(void) {
	uInt8 byte;
	size_t n = read(spi_fd, &byte, sizeof(byte));
	if (n == 1)
		printf("SpiReadByte           %08x\n", byte);
	else
		printf("SpiReadByte  failed\n");
	return byte;
}

void SpiWriteByte(uInt8 wrData) {
	size_t n = write(spi_fd, &wrData, sizeof(wrData));
	if (n == 1)
		printf("SpiWriteByte %08x\n", wrData);
	else
		printf("SpiWriteByte failed\n");
}

int SPI_Init (ctrl_S * hSpi) {
	//printf("SPI_Init\n");
	char* errmsg = 0;

	if ((spi_fd = open(device, O_RDWR)) == -1) { // O_NONBLOCK?
		errmsg = "can't open device";
	}
	else	if ((ser_fd = openserial(serdevice)) == -1) {
		errmsg = "can't open serial device";
	}
	else {

		if (-1 == ioctl(spi_fd, SPI_IOC_WR_MODE, &mode)) {
			errmsg = "can't set spi mode";
		}
		else if (-1 == ioctl(spi_fd, SPI_IOC_RD_MODE, &mode)) {
			errmsg = "can't get spi mode";
		}
		else if (-1 == ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits)) {
			errmsg = "can't set bits per word";
		}
		else if (-1 == ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits)) {
			errmsg = "can't get bits per word";
		}
		else if (-1 == ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)) {
			errmsg = "can't set max speed hz";
		}
		else if (-1 == ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)) {
			errmsg = "can't get max speed hz";
		}
		else {

			//printf("spi mode: %d\n", mode);
			//printf("bits per word: %d\n", bits);
			//printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

			setRTS(ser_fd, 0);

			return 0;
		}
	}
	printf("%s, errno=%d\n", errmsg, errno);
	return -1;
}

void SPI_Deinit()
{
	close(spi_fd);
	closeserial(ser_fd);
}

/*
** Function: spiGci_ResetWrapper
**
** Description: 
** Sets the reset pin of the ProSLIC
*/
int ctrl_ResetWrapper (void * hSpiGci, int status){
	printf("ctrl_ResetWrapper %d\n", status);
	setRTS(ser_fd, status ? 1 : 0);
	return 0;
}

/*
** chanNumToCID()
**
** Convert channel # to CID word
**
*/
uInt8 chanNumToCID(uInt8 channelNum) {
	uInt8 cid;
	cid = (channelNum<<4) & 0x10;
	cid |= (channelNum<<2) & 0x08;
	cid |= (channelNum>>2) & 0x02;
	cid |= (channelNum>>4) & 0x01;
	cid |= channelNum & 0x04;
	return cid;
}

/*
** SPI/GCI register read 
**
** Description: 
** Reads a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** num: number of reads to perform
** regAddr: Address of register to read
** addr_inc: whether to increment address after each read
** data: data to read from register
**
** Return:
** none
*/
uInt8 ctrl_ReadRegisterWrapper (void * hSpiGci, uInt8 channel, uInt8 regAddr){
	uInt8 controlWord;

	if(channel == 0xFF) { /* Broadcast */
		controlWord = 0x80;
	}
	else {
		controlWord = chanNumToCID(channel); /* Encode CID */
	}
	controlWord |= 0x60;
	/* Set R/W and REG/RAM */

	/*
	SpiWriteByte(controlWord);
	SpiWriteByte(regAddr);
	return SpiReadByte();
	*/

	// /CS is deasserted for at least 220ns between each 8-bit tranfer

	uInt8 byte = 0;

	struct spi_ioc_transfer xfer[3] = {
		{
			.tx_buf = (unsigned long)&controlWord,
			.len = 1,
			.cs_change = 1
		},
		{
			.tx_buf = (unsigned long)&regAddr,
			.len = 1,
			.cs_change = 1
		},
		{
			.rx_buf = (unsigned long)&byte,
			.len = 1,
		}
	};

	int status = ioctl(spi_fd, SPI_IOC_MESSAGE(3), xfer);

	if (status < 0) {
		printf("ctrl_ReadRegisterWrapper error %d errno %d\n", status, errno);
	}
	else
		printf("ctrl_ReadRegisterWrapper           %02x %02x %02x\n", controlWord, regAddr,  byte);

	return byte;
}


/*
** Function: spiGci_WriteRegisterWrapper 
**
** Description: 
** Writes a single ProSLIC register
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of register to write
** data: data to write to register
**
** Return:
** none
*/
int ctrl_WriteRegisterWrapper (void * hSpiGci, uInt8 channel,  uInt8 regAddr, uInt8 data){
	
	uInt8 controlWord;

	if(channel == 0xFF) { /* Broadcast */
		controlWord = 0x80;
	}
	else {
		controlWord = chanNumToCID(channel); /* Encode CID */
	}
	controlWord |= 0x20;
	controlWord &= 0xBF;
	/* Set REG/RAM */
	/* Clear R/W */

//	SpiWriteByte(controlWord);
//	SpiWriteByte(regAddr);
//	SpiWriteByte(data);

	struct spi_ioc_transfer xfer[3] = {
		{
			.tx_buf = (unsigned long)&controlWord,
			.len = 1,
			.cs_change = 1
		},
		{
			.tx_buf = (unsigned long)&regAddr,
			.len = 1,
			.cs_change = 1
		},
		{
			.tx_buf = (unsigned long)&data,
			.len = 1,
		}
	};

	int status = ioctl(spi_fd, SPI_IOC_MESSAGE(3), xfer);

	if (status < 0) {
		printf("ctrl_WriteRegisterWrapper error %d errno %d\n", status, errno);
	}
	else
		printf("ctrl_WriteRegisterWrapper  %02x %02x %02x\n", controlWord, regAddr,  data);

	ctrl_ReadRegisterWrapper (hSpiGci, channel, regAddr);

	return 0;
}


/*
** Function: SPI_ReadRAMWrapper
**
** Description: 
** Reads a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to read from
** address: Address of RAM location to read
** pData: data to read from RAM location
**
** Return:
** none
*/
uInt32 ctrl_ReadRAMWrapper (void *hCtrl, uInt8 channel, uInt16 ramAddr){
	return 0xAAAAAAAAL;
}


/*
** Function: SPI_WriteRAMWrapper
**
** Description: 
** Writes a single ProSLIC RAM location
**
** Input Parameters: 
** channel: ProSLIC channel to write to
** address: Address of RAM location to write
** data: data to write to RAM location
**
** Return:
** none
*/
int ctrl_WriteRAMWrapper (void *hCtrl, uInt8 channel, uInt16 ramAddr, ramData data){

	return 0;
}



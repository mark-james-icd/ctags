OBJS = \
	slic.o \
	spi_intf.o \
	timer_intf.o \
	\
	proslic_api/src/proslic_tstin.o \
	proslic_api/src/proslic.o \
	proslic_api/src/si_voice_version.o \
	proslic_api/src/si_voice.o \
	proslic_api/src/si3217x_intf.o \
	proslic_api/src/si3217x_revb_intf.o \
	proslic_api/src/si3217x_revc_intf.o \
	proslic_api/config_src/proslic_tstin_limits.o \
	proslic_api/config_src/si3217x_BKBT_constants.o \
	proslic_api/patch_files/si3217x_patch_C_FB_2013FEB04.o \
	proslic_api/patch_files/si3217x_patch_B_FB_2012DEC10.o

INCS = -I. -Iproslic_api/inc -Iproslic_api/config_inc
	
CC = arm-linux-gnueabihf-gcc 
CCFLAGS = -march=armv7 $(INCS)



%.o : %.c
	$(CC) $(CCFLAGS) -g -o $@ -c $<	

TARGET = slic

all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) -g -o $@ $^ -lrt

clean:
	rm -f $(OBJS) $(TARGET)
	

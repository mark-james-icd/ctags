CC=arm-linux-gnueabihf-gcc -march=armv7
#CC=gcc
#CFLAGS=-I./include -I../inc -DLR8
CFLAGS=-I./include -I../inc -DLR16

DEPS=packet_arbiter.h rcu_api.h
LIBS=-lpthread
OBJ=main.o server.o ir.o utils.o packet_mux.o lightring_animation.o flasher.o cobs.o event.o serial_port.o
SRC=main.c server.c ir.c utils.c packet_mux.c lightring_animation.c flasher.c event.c cobs.c serial_port.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mfddrv: $(OBJ)
	$(CC) -o mfddrv main.o server.o ir.o packet_mux.o utils.o lightring_animation.o flasher.o cobs.o event.o serial_port.o -lpthread  $(CFLAGS)



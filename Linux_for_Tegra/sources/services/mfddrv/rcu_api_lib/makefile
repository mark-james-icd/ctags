CC=arm-linux-gnueabihf-gcc -march=armv7
#CC=gcc
#CFLAGS=-I ../inc -DLR8
CFLAGS=-I ../inc -DLR16
DEPS=packet_arbiter.h rcu_api.h

all: rcu_api makelib

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rcu_api: rcu_api.c
	$(CC) -c rcu_api.c $(CFLAGS)

makelib: rcu_api.o
	ar rcs librcu_api.a rcu_api.o







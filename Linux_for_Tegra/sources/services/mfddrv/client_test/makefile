CC=arm-linux-gnueabihf-gcc -march=armv7
#CC=gcc
#CFLAGS=-I ../inc -L ../rcu_api_lib -DLR8
CFLAGS=-I ../inc -L ../rcu_api_lib -DLR16
DEPS=packet_arbiter.h rcu_api.h client.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

clienttest: client.o ../rcu_api_lib/librcu_api.a
	$(CC) -o clienttest client.o -lrcu_api $(CFLAGS)



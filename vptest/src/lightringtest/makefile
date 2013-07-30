CC=arm-linux-gnueabihf-gcc -march=armv7
#CC=gcc
CFLAGS=-I ../../../Linux_for_Tegra/sources/services/mfddrv/inc -L ../../../Linux_for_Tegra/sources/services/mfddrv/rcu_api_lib -DLR8
#CFLAGS=-I ../inc -L ../../../Linux_for_Tegra/sources/services/mfddrv/rcu_api_lib -DLR16
DEPS=packet_arbiter.h rcu_api.h lightringtest.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

clienttest: lightringtest.o ../../../Linux_for_Tegra/sources/services/mfddrv/rcu_api_lib/librcu_api.a
	$(CC) -o lightringtest lightringtest.o -lrcu_api $(CFLAGS)




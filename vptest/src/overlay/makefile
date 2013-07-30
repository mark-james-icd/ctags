CC=gcc
#CC=arm-linux-gnueabihf-gcc -march=armv7
CFLAGS=-I.
PKG=`pkg-config --cflags --libs gstreamer-0.10 gtk+-x11-3.0`
DEPS = 
OBJ = overlay.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(PKG) $(CFLAGS)

overlay: $(OBJ)
	gcc -o $@ $^ $(PKG) $(CFLAGS)

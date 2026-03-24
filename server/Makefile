# For cross compilation use:
# make CROSS_COMPILE=aarch64-none-linux-gnu-

CC=$(CROSS_COMPILE)gcc

aesdsocket: aesdsocket.o
	$(CC) $< -o $@

aesdsocket.o: aesdsocket.c
	$(CC) -c $< -o $@

all: aesdsocket

default: aesdsocket

clean:
	rm -f  *.o aesdsocket a.out

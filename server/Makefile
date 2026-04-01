# For cross compilation use:
# make CROSS_COMPILE=aarch64-none-linux-gnu-

CC=$(CROSS_COMPILE)gcc

aesdsocket: aesdsocket.o thread_slist.o
	$(CC) $< -o $@

aesdsocket.o: aesdsocket.c
	$(CC) -c $< -o $@

llisttest: llisttest.o
	$(CC)  $< -o $@

llisttest.o: llisttest.c
	$(CC) -c $< -o $@

thread_slist.o: thread_slist.c thread_slist.h
	$(CC) -c $< -o $@

all: aesdsocket

default: aesdsocket

clean:
	rm -f  *.o aesdsocket llisttest a.out

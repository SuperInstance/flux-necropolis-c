CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
AR = ar

all: libnecropolis.a test

libnecropolis.a: necropolis.o
	$(AR) rcs $@ $^

necropolis.o: necropolis.c necropolis.h
	$(CC) $(CFLAGS) -c necropolis.c -o $@

test: test_necropolis.c necropolis.h libnecropolis.a
	$(CC) $(CFLAGS) test_necropolis.c libnecropolis.a -o $@
	./test

test_necropolis: test_necropolis.c necropolis.h

clean:
	rm -f *.o *.a test

.PHONY: all clean

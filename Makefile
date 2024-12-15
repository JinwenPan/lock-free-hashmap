CC ?= gcc
CFLAGS ?= -g -Wall -O2

.PHONY: all clean

all: liblockfreehashmap.so

liblockfreehashmap.so: lockfreehashmap.c
	$(CC) $(CFLAGS) -shared -fPIC -ldl -o $@ $<

clean:
	rm -rf *.so* *.o
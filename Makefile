AR ?= ar
CC ?= gcc
CFLAGS = -std=c99 -Wall -Wextra -fpic
PREFIX ?= /usr/local
SRCS += optvalues.c optparser.c
OBJS += $(SRCS:.c=.o)

ifeq ($(DEBUG), true)
	CFLAGS += -g -O0
else
	CFLAGS += -O2
endif

all:build

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

build:liboptparser.a liboptparser.so

liboptparser.a:$(OBJS)
	echo $(OBJS)
	$(AR) -crs $@ $^

liboptparser.so:$(OBJS)
	$(CC) $(CFLAGS) -shared $^ -o $@

test:test.o liboptparser.a
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o *.dSYM *.a *.so test

install:
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp -f optparser.h $(PREFIX)/include/optparser.h
	cp -f liboptparser.a $(PREFIX)/lib/liboptparser.a
	cp -f liboptparser.so $(PREFIX)/lib/liboptparser.so

uninstall:
	rm -rf $(PREFIX)/include/optparser.h
	rm -rf $(PREFIX)/lib/liboptparser.a
	rm -rf $(PREFIX)/lib/liboptparser.so

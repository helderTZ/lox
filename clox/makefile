CC=gcc
CFLAGS=-W -Wall -Wextra -Wno-unused-parameter -Wno-sequence-point
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

ifneq ($(DEBUG),)
CFLAGS += -g -O0
endif

all: clox

clox: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm clox
	rm -f $(OBJECTS)
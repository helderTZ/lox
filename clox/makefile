CC=gcc
CFLAGS=-W -Wall -Wextra -Wno-unused-parameter
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

all: clox

clox: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm clox
	rm -f $(OBJECTS)
CC=gcc
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

all: clox

clox: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(OBJECTS): %.o: %.c
	$(CC) -c $< -o $@

.PHONY: clean

clean:
	rm clox
	rm -f $(OBJECTS)
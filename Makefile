CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

all: main.o parser.o responder.o
	$(CC) -o server $^ $(CFLAGS)

%.o: %.c utils.h 
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf *.o
distclean:
	rm -rf *.o server

NAME = usbnotify
CC = gcc
CFLAGS = -Wall -O3 $(shell pkg-config --cflags libnotify)
LDFLAGS = $(shell pkg-config --libs libnotify)

all: main.o 
	$(CC) $(LDFLAGS) main.o -o $(NAME)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm main.o $(NAME)

install:
	cp $(NAME) ~/.local/bin

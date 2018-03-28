CC?=gcc

all:
	$(CC) -Wall -std=c99 -o roll main.c -DSHOW_ROLLS
dont_show_rolls:
	$(CC) -Wall -std=c99 -o roll main.c

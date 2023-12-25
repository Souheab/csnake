
CC=gcc

all: snake

snake: snake.c
	$(CC) -o snake snake.c -lncurses

clean:
	rm snake



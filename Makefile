
CC=gcc

all: snake

snake: snake.c
	$(CC) -lncurses -o snake snake.c

clean:
	rm snake



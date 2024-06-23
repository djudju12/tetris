build: tetris.c
	gcc -o tetris tetris.c -Wall -Wextra -lraylib -lm -ggdb

run: build
	./tetris
build: tetris.c
	gcc -o tetris tetris.c -Wall -Wextra -lraylib -ggdb

run: build
	./tetris
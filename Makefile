RAYLIB_PATH=./raylib/raylib-5.0/src

CC=gcc
FLAGS=-Wall -Wextra -ggdb -I$(RAYLIB_PATH)
LDFLAGS=-L$(RAYLIB_PATH) -lraylib -lm
OUT=bin

.PHONY: clean

build: build/raylib bin/ tetris.c
	$(CC) $(FLAGS) -o $(OUT)/tetris tetris.c $(LDFLAGS)

build/raylib:
	$(MAKE) -C $(RAYLIB_PATH)

clean:
	$(MAKE) -C $(RAYLIB_PATH) clean
	if [ -d $(OUT) ]; then rm -r $(OUT); fi

run: build
	./tetris

bin/:
	mkdir -p bin
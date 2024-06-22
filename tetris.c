#include <stdio.h>

#include "raylib.h"

#define GRID_WIDTH    350
#define GRID_HEIGHT   GRID_WIDTH*2
#define CELL_SIZE     GRID_WIDTH/10
#define GRID_PAD      CELL_SIZE
#define SCREEN_WIDTH  GRID_WIDTH  + GRID_PAD
#define SCREEN_HEIGHT GRID_HEIGHT + GRID_PAD
#define GRID_ROWS     10
#define GRID_COLS     20

#define VEC(xx, yy) (Vector2) {.x = (xx), .y = (yy)}
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define out_of_bounds(x, y) ((x) < 0 || (x) >= GRID_ROWS || (y) < 0 || (y) >= GRID_COLS)

int vel_y = 50;
float t = 0.0;

int grid[GRID_ROWS][GRID_COLS] = {0};

Vector2 grid_to_world(int x, int y) {
    return VEC(x*CELL_SIZE + GRID_PAD/2, y*CELL_SIZE + GRID_PAD/2);
}

#define PSIZE 4

typedef struct {
    int type;
    int rotation;
    int x, y; // grid scale
} Block;

Block block = {0}; // One block at time

int patterns[][PSIZE*4][PSIZE] = {
    {
        {1, 1, 2, 2},
        {1, 1, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},

        {1, 1, 2, 2},
        {1, 1, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},

        {1, 1, 2, 2},
        {1, 1, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},

        {1, 1, 2, 2},
        {1, 1, 2, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},
    },
    {
        {2, 1, 2, 2},
        {2, 1, 2, 2},
        {2, 1, 2, 2},
        {2, 1, 2, 2},

        {2, 2, 2, 2},
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {2, 2, 2, 2},

        {2, 1, 2, 2},
        {2, 1, 2, 2},
        {2, 1, 2, 2},
        {2, 1, 2, 2},

        {2, 2, 2, 2},
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {2, 2, 2, 2},
    },
    {
        {2, 2, 2, 2},
        {1, 1, 1, 2},
        {2, 1, 2, 2},
        {2, 2, 2, 2},

        {2, 1, 2, 2},
        {2, 1, 1, 2},
        {2, 1, 2, 2},
        {2, 2, 2, 2},

        {2, 1, 2, 2},
        {1, 1, 2, 2},
        {2, 1, 2, 2},
        {2, 2, 2, 2},

        {2, 1, 2, 2},
        {1, 1, 1, 2},
        {2, 2, 2, 2},
        {2, 2, 2, 2},
    },
};

int is_move_possible(int x, int y) {
    return !(out_of_bounds(block.x + x, block.y + y) && patterns[block.type][block.rotation*4 + y][x] == 1);
}

void rotate_block() {
    if (IsKeyPressed(KEY_UP)) {
        const int last_rot = block.rotation;
        block.rotation += 1;
        block.rotation %= 4;

        for (int x = 0 ; x < PSIZE; x++) {
            for (int y = 0; y < PSIZE; y++) {
                if (!is_move_possible(x, y)) {
                    block.rotation = last_rot;
                    break;
                }
            }
        }
    }
}

void move_block() {
    /* not implemented */
    const int last_x = block.x;
    const int last_y = block.y;

    if (IsKeyPressed(KEY_LEFT)) {
        block.x -= 1;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        block.x += 1;
    }


    t += GetFrameTime() * 1000;
    if (t >= vel_y) {
        t = 0;
        block.y += 1;
    }

    for (int x = 0 ; x < PSIZE; x++) {
        for (int y = 0; y < PSIZE; y++) {
            if (!is_move_possible(x, y)) {
                TraceLog(LOG_INFO, "%d", block.y + y);
                if (block.y + y == GRID_ROWS) {
                    block.y = 0;
                } else {
                    block.x = last_x;
                    block.y = last_y;
                }
                break;
            }
        }
    }

    // if (!is_move_possible(block.x, block.y)) {
    //     // new_block();
    //     block.y = 0;
    // }
}

void set_block() {
    for (int x = 0; x < GRID_ROWS; x++) {
        for (int y = 0; y < GRID_COLS; y++) {
            grid[x][y] = 0;
        }
    }

    const int rot_offset = block.rotation*4;
    for (int x = 0 ; x < PSIZE; x++) {
        for (int y = 0; y < PSIZE; y++) {
            int nx = block.x + x;
            int ny = block.y + y;
            if (!out_of_bounds(nx, ny)) {
                grid[nx][ny] = patterns[block.type][rot_offset + y][x];
            }
        }
    }
}

void draw_cells() {
    for (int x = 0; x < GRID_ROWS; x++) {
        for (int y = 0; y < GRID_COLS; y++) {
            Vector2 pos = grid_to_world(x, y);
            if (grid[x][y] == 1) {
                DrawRectangleV(pos, VEC(CELL_SIZE, CELL_SIZE), PURPLE);
            }
            // else if (grid[x][y] == 2) {
            //     DrawRectangleV(pos, VEC(CELL_SIZE, CELL_SIZE), LIME);
            // }
        }
    }
}

void draw_grid() {
    const int x0 = GRID_PAD/2;
    const int y0 = GRID_PAD/2;
    for (int x = 0; x <= GRID_ROWS; x++) {
        DrawLine(x*CELL_SIZE + x0, y0, x*CELL_SIZE + x0, GRID_HEIGHT + y0, WHITE);
    }

    for (int y = 0; y <= GRID_COLS; y++) {
        DrawLine(x0, y*CELL_SIZE + y0, GRID_WIDTH + x0, y*CELL_SIZE + y0, WHITE);
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tetris");

    block.type = 0;
    block.x = 0;
    while (!WindowShouldClose()) {
        // updates
        rotate_block();
        move_block();
        set_block();

        // draws
        BeginDrawing();
        {
            ClearBackground(BLACK);
            draw_cells();
            draw_grid();
        }
        EndDrawing();
    }

    CloseWindow();
}
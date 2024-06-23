#include <stdio.h>

#include "raylib.h"

#define GRID_WIDTH    350
#define GRID_HEIGHT   GRID_WIDTH*2
#define CELL_SIZE     GRID_WIDTH/10
#define GRID_PAD      CELL_SIZE
#define SCREEN_WIDTH  GRID_WIDTH  + GRID_PAD
#define SCREEN_HEIGHT GRID_HEIGHT + GRID_PAD
#define GRID_ROWS     20
#define GRID_COLS     10

#define VEC(xx, yy) (Vector2) {.x = (xx), .y = (yy)}
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define out_of_bounds(x, y) ((x) < 0 || (x) >= GRID_COLS || (y) < 0 || (y) >= GRID_ROWS)

int vel_y = 500;
float t = 0.0;

int grid[GRID_ROWS][GRID_COLS] = {0};

Vector2 grid_to_world(int x, int y) {
    return VEC(x*CELL_SIZE + GRID_PAD/2, y*CELL_SIZE + GRID_PAD/2);
}

enum {
    EMPTY = 0,
    BLOCK    ,
    OCCUPIED ,
} Cell_State;

#define PSIZE 4

typedef struct {
    int type;
    int rotation;
    int x, y; // grid scale
} Block;

Block block = {0}; // One block at time

#define PATTERNS_COUNT 7
int patterns[PATTERNS_COUNT][PSIZE*4][PSIZE] = {
    { // square
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // line
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},

        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
    },
    { // revolver 1
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},

        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
    },
    { // revolver 2
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},

        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0},

        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    },
    { // torneira
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},

        {0, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    },
    { // skibidi 1
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 0, 1, 0},
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},

        {0, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0},
    },
    { // skibidi 2
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},

        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},

        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},

        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0},
    }
};

#define CIAN
Color patterns_colors[PATTERNS_COUNT] = {
//  square, line   , revolver 1, revolver 2, torneira, skibidi 1, skibidi 2
    YELLOW, SKYBLUE, BLUE      , ORANGE    , PURPLE  , RED      , GREEN
};

int is_move_possible(int x, int y) {
    int cell_state = patterns[block.type][block.rotation*4 + y][x];
    int nx = block.x + x;
    int ny = block.y + y;
    // move is possible = out of bounds? cell cannot be BLOCK
    // if this move is possible, we have to check if the cell
    // that we are trying to visit with block inst already occupied
    return !(out_of_bounds(nx, ny) && cell_state == BLOCK) && !(grid[ny][nx] == OCCUPIED && cell_state == BLOCK);
}

int block_in_impossible_state() {
    for (int x = 0 ; x < PSIZE; x++) {
        for (int y = 0; y < PSIZE; y++) {
            if (!is_move_possible(x, y)) {
                return 1;
            }
        }
    }

    return 0;
}

void rotate_block() {
    if (IsKeyPressed(KEY_UP)) {
        const int last_rot = block.rotation;
        block.rotation += 1;
        block.rotation %= 4;

        if (block_in_impossible_state()) {
            block.rotation = last_rot;
        }
    }
}

void check_points() {
    for (int y = 0; y < GRID_ROWS; y++) {
        int cnt = 0;
        for (int x = 0; x < GRID_COLS; x++) {
            if (grid[y][x] == OCCUPIED) cnt += 1;
        }

        if (cnt != GRID_COLS) continue;
        for (int y2 = y; y2 > 0; y2--) {
            for (int x = 0; x < GRID_COLS; x++) {
                grid[y2][x] = grid[y2 - 1][x];
            }
        }
    }
}

void move_block() {
    const int last_x = block.x;

    if (IsKeyPressed(KEY_LEFT)) {
        block.x -= 1;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        block.x += 1;
    }

    if (block_in_impossible_state()) {
        block.x = last_x;
    }

    if (IsKeyDown(KEY_DOWN)) {
        vel_y = 500/10;
    }

    if (IsKeyUp(KEY_DOWN)) {
        vel_y = 500;
    }

    t += GetFrameTime() * 1000;
    if (t >= vel_y) {
        t = 0;
        block.y += 1;
        if (block_in_impossible_state()) {
            block.y -= 1;
            for (int x = 0 ; x < PSIZE; x++) {
                for (int y = 0; y < PSIZE; y++) {
                    int nx = block.x + x;
                    int ny = block.y + y;
                    grid[ny][nx] = patterns[block.type][block.rotation*4 + y][x] == BLOCK ? OCCUPIED : grid[ny][nx];
                }
            }

            block.y = 0;
            block.x = GRID_COLS/2;
            block.type += 1;
            block.type %= PATTERNS_COUNT;
            check_points();
        }
    }
}

void set_block() {
    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            if (grid[y][x] == BLOCK) {
                grid[y][x] = EMPTY;
            }
        }
    }

    const int rot_offset = block.rotation*4;
    for (int x = 0 ; x < PSIZE; x++) {
        for (int y = 0; y < PSIZE; y++) {
            int nx = block.x + x;
            int ny = block.y + y;
            if (!out_of_bounds(nx, ny)) {
                if (patterns[block.type][rot_offset + y][x] == BLOCK) {
                   grid[ny][nx] = patterns[block.type][rot_offset + y][x];
                }
            }
        }
    }
}

void draw_cells() {
    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            Vector2 pos = grid_to_world(x, y);
            Rectangle r = (Rectangle) {
                .height = CELL_SIZE,
                .width = CELL_SIZE,
                .x = pos.x,
                .y = pos.y
            };

            if (grid[y][x] == BLOCK) {
                DrawRectangleRec(r, patterns_colors[block.type]);
            } else if (grid[y][x] == OCCUPIED) {
                DrawRectangleRec(r, WHITE);
                DrawRectangleLinesEx(r, 1, BLACK);
            }
        }
    }
}

void draw_grid() {
    const int x0 = GRID_PAD/2;
    const int y0 = GRID_PAD/2;
    for (int x = 0; x <= GRID_COLS; x++) {
        DrawLine(x*CELL_SIZE + x0, y0, x*CELL_SIZE + x0, GRID_HEIGHT + y0, WHITE);
    }

    for (int y = 0; y <= GRID_ROWS; y++) {
        DrawLine(x0, y*CELL_SIZE + y0, GRID_WIDTH + x0, y*CELL_SIZE + y0, WHITE);
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tetris");

    block.type = 2;
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
#include <stdio.h>
#include <time.h>

#include "raylib.h"

#define GRID_WIDTH    350
#define GRID_HEIGHT   (GRID_WIDTH*2)
#define CELL_SIZE     (GRID_WIDTH/10)
#define GRID_PAD      CELL_SIZE
#define SCREEN_WIDTH  (GRID_WIDTH  + GRID_PAD)
#define SCREEN_HEIGHT (GRID_HEIGHT + GRID_PAD)
#define GRID_ROWS     20
#define GRID_COLS     10
#define VEL_Y         500

#define VEC(xx, yy) (Vector2) {.x = (xx), .y = (yy)}
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define out_of_bounds(x, y) ((x) < 0 || (x) >= GRID_COLS || (y) < 0 || (y) >= GRID_ROWS)

struct Game {
    int vel_y;
    int over;
    float t;
    int points;
    int grid[GRID_ROWS][GRID_COLS];
};

struct Game game = {0};

Vector2 grid_to_world(int x, int y) {
    return (Vector2) {
        .x = x*CELL_SIZE + GRID_PAD/2,
        .y = y*CELL_SIZE + GRID_PAD/2
    };
}

enum {
    EMPTY = 0,
    BLOCK    ,
    OCCUPIED ,
} Cell_State;

typedef struct {
    int type;
    int rotation;
    int x, y; // grid scale
} Block;

Block block = {0}; // One block at time

#define PATTERNS_COUNT 7
#define PSIZE 4

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

Color patterns_colors[PATTERNS_COUNT] = {
//  square, line   , revolver 1, revolver 2, torneira, skibidi 1, skibidi 2
    YELLOW, SKYBLUE, BLUE      , ORANGE    , PURPLE  , RED      , GREEN
};

typedef enum {
    MOVE_OK = 1          ,
    MOVE_OUT_OF_BOUNDS   ,
    MOVE_ALREADY_OCCUPIED,
} Move_Result;

Move_Result check_cell(int x, int y) {
    int cell_state = patterns[block.type][block.rotation*4 + y][x];
    int nx = block.x + x;
    int ny = block.y + y;
    if (out_of_bounds(nx, ny) && cell_state == BLOCK) {
        return MOVE_OUT_OF_BOUNDS;
    } else if (game.grid[ny][nx] == OCCUPIED && cell_state == BLOCK) {
        return MOVE_ALREADY_OCCUPIED;
    } else {
        return MOVE_OK;
    }
}

Move_Result do_move() {
    for (int x = 0 ; x < PSIZE; x++) {
        for (int y = 0; y < PSIZE; y++) {
            Move_Result result = check_cell(x, y);
            if (result != MOVE_OK) {
                return result;
            }
        }
    }

    return MOVE_OK;
}

void rotate_block() {
    if (IsKeyPressed(KEY_UP)) {
        const int last_rot = block.rotation;
        block.rotation += 1;
        block.rotation %= 4;

        if (do_move() != MOVE_OK) {
            block.rotation = last_rot;
        }
    }
}

void check_points() {
    for (int y = 0; y < GRID_ROWS; y++) {
        int cnt = 0;
        for (int x = 0; x < GRID_COLS; x++) {
            if (game.grid[y][x] == OCCUPIED) cnt += 1;
        }

        if (cnt == GRID_COLS) {
            game.points += 1;
            for (int y2 = y; y2 > 0; y2--) {
                for (int x = 0; x < GRID_COLS; x++) {
                    game.grid[y2][x] = game.grid[y2 - 1][x];
                }
            }
        }
    }
}

void new_block() {
    block.y = 0;
    block.x = GRID_COLS/2 - 1;
    block.type = GetRandomValue(0, PATTERNS_COUNT - 1);
}

void move_block() {
    const int last_x = block.x;

    if (IsKeyPressed(KEY_LEFT)) {
        block.x -= 1;
    }

    if (IsKeyPressed(KEY_RIGHT)) {
        block.x += 1;
    }

    if (do_move() != MOVE_OK) {
        block.x = last_x;
    }

    if (IsKeyDown(KEY_DOWN)) {
        game.vel_y = VEL_Y/10;
    }

    if (IsKeyUp(KEY_DOWN)) {
        game.vel_y = VEL_Y;
    }

    game.t += GetFrameTime() * 1000;
    if (game.t >= game.vel_y) {
        game.t = 0;
        block.y += 1;
        Move_Result result = do_move();
        if (result != MOVE_OK) {
            block.y -= 1;
            for (int x = 0 ; x < PSIZE; x++) {
                for (int y = 0; y < PSIZE; y++) {
                    int nx = block.x + x;
                    int ny = block.y + y;
                    game.grid[ny][nx] = patterns[block.type][block.rotation*4 + y][x] == BLOCK ? OCCUPIED : game.grid[ny][nx];
                }
            }

            new_block();
            check_points();
            if (do_move() != MOVE_OK) {
                game.over = 1;
            }
        }
    }
}

void set_block() {
    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            if (game.grid[y][x] == BLOCK) {
                game.grid[y][x] = EMPTY;
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
                   game.grid[ny][nx] = patterns[block.type][rot_offset + y][x];
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

            if (game.grid[y][x] == BLOCK) {
                DrawRectangleRec(r, patterns_colors[block.type]);
            } else if (game.grid[y][x] == OCCUPIED) {
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

void draw_game_over() {
    const int font_size = 48;
    const int y0 = GRID_ROWS/2 - 2;
    const int padx = 27;
    const Color font_color = LIME;
    char buff[32];
    sprintf(buff, "%d", game.points);

    Vector2 over   = grid_to_world(0, y0 - 3);
    Vector2 score  = grid_to_world(0, y0 - 1);
    Vector2 points = grid_to_world(0, y0 + 1);

    DrawText("GAME OVER!", over.x   + padx, over.y  , font_size, font_color);
    DrawText("SCORE:"    , score.x  + padx, score.y , font_size, font_color);
    DrawText(buff        , points.x + padx, points.y, font_size, font_color);
}

void init_game() {
    for (int y = 0; y < GRID_ROWS; y++) {
        for (int x = 0; x < GRID_COLS; x++) {
            game.grid[y][x] = EMPTY;
        }
    }

    SetRandomSeed(time(NULL));

    game.points = 0;
    game.over   = 0;
    game.t      = 0;
    game.vel_y  = VEL_Y;
    new_block();
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tetris");

    init_game();
    while (!WindowShouldClose()) {
        // updates
        if (game.over) {
            if (IsKeyPressed(KEY_SPACE)) {
                init_game();
            }
        } else {
            rotate_block();
            move_block();
            set_block();
        }

        // draws
        BeginDrawing();
        {
            ClearBackground(BLACK);
            draw_cells();
            draw_grid();
            if (game.over) {
                draw_game_over();
            }
        }
        EndDrawing();
    }

    CloseWindow();
}
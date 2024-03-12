#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define C_BASIC  (Color) { 0x50, 0x50, 0x50, 0xff }
#define C_DARKER (Color) { 0x20, 0x20, 0x20, 0xff }
#define C_HOVER  (Color) { 0x40, 0x40, 0x60, 0xff }

#define TILE_OPEN          0
#define IS_MINE            1
#define FLAGGED            2
#define FLAGGED_WRONG      3
#define MINES_IN_NEIGHBORS 4 // 4-bits

#define  CHECK_BIT(v, b) !!((v) &   (1 << (b))) 
#define    SET_BIT(v, b)    (v) |=  (1 << (b))
#define TOGGLE_BIT(v, b)    (v) ^=  (1 << (b))
#define  CLEAR_BIT(v, b)    (v) &= ~(1 << (b))
#define GET_NEIGHBOR_VALUE(d) (((d)>>MINES_IN_NEIGHBORS) & 0xf)

typedef struct Level {
    uint16_t width;
    uint16_t height;
    uint32_t capacity;
    uint32_t closed_tiles;
    uint16_t mine_amount;
    uint8_t *data;
} Level;

void 
set_data_to_zero(Level *level)
{
    uint32_t i;

    for (i = 0; i < level->capacity; ++i){
        level->data[i] = 0;
    }
}

uint8_t  
count_mines_in_neighbors(Level *l, uint32_t tile)
{
    uint8_t ret = 0;
    div_t pos = div(tile, l->width);

    int32_t t_pos;

    if (pos.quot > 0){                  
        t_pos = tile - l->width;        
        if (pos.rem > 0 && CHECK_BIT(l->data[t_pos - 1], IS_MINE)) ret++;
        if (CHECK_BIT(l->data[t_pos], IS_MINE)) ret++;
        if (pos.rem < l->width - 1 && CHECK_BIT(l->data[t_pos + 1], IS_MINE)) ret++;
    }

    if (pos.rem > 0 && CHECK_BIT(l->data[tile - 1], IS_MINE)) ret++; 
    if (pos.rem < l->width - 1 && CHECK_BIT(l->data[tile + 1], IS_MINE)) ret++;

    if (pos.quot < l->height - 1){
        t_pos = tile + l->width;
        if (pos.rem > 0 && CHECK_BIT(l->data[t_pos - 1], IS_MINE)) ret++;
        if (CHECK_BIT(l->data[t_pos], IS_MINE)) ret++;
        if (pos.rem < l->width - 1 && CHECK_BIT(l->data[t_pos + 1], IS_MINE)) ret++;
    }
    return ret;
}

void 
free_level(Level *level)
{
    free(level->data);
}

void 
set_mines(Level *level)
{
    int mine_ct = 0;
    for (;;) {
        int tile = rand() % level->capacity;
        if (!CHECK_BIT(level->data[tile], IS_MINE)){
            SET_BIT(level->data[tile], IS_MINE);
            if (++mine_ct == level->mine_amount) break;      
        }
    }

}

void 
initialize_level(Level *level, 
                 uint16_t width, uint16_t height, 
                 uint16_t mine_amount)
{

    level->width = width;
    level->height = height;
    level->capacity = width * height;
    level->closed_tiles = level->capacity;
    level->mine_amount = mine_amount;

    level->data = (uint8_t*) malloc(sizeof(uint8_t) * width * height);
    if (level->data == NULL) {
        exit(1);
    }
    set_data_to_zero(level);
    set_mines(level);
    
    uint8_t mines_count;
    for (uint32_t i = 0; i < level->capacity; ++i){
        if (CHECK_BIT(level->data[i], IS_MINE)) continue;
        mines_count = count_mines_in_neighbors(level, i);
        level->data[i] |= (mines_count << MINES_IN_NEIGHBORS);
    }

    //printf("W: %d\nH: %d\nC:%d\n", level->width, level->height, level->capacity);

}

void 
show_tile(int x, int y, int w, int h, uint8_t data, uint8_t hovered)
{
    if (!CHECK_BIT(data, TILE_OPEN)){
        DrawRectangle(x, y, w, h, hovered ? C_HOVER : C_DARKER);
        DrawLine(x, y, x + w, y, WHITE);
        DrawLine(x, y, x, y + h, WHITE); 
        DrawLine(x + w - 1, y + h - 1, x + w, y, C_DARKER);
        DrawLine(x + w - 1, y + h - 1, x, y + h, C_DARKER);

        if (!CHECK_BIT(data, FLAGGED)) return;
        int offset = 10;
        DrawRectangle(x + offset, y + offset, w - offset * 2, h - offset * 2, RED); 
        return;
    }

    DrawRectangle(x, y, w, h, C_BASIC);
    
    if (CHECK_BIT(data, IS_MINE)){
        DrawCircle(x + (w >> 1), y + (h >> 1), 10, RED);
        return;
    }
    
    uint8_t neighbors_amount = (data >> 4) & 0xf;
    if (neighbors_amount){
        char number[3];
        sprintf(number, "%d", neighbors_amount);
        DrawText(number, x + (w >> 2), y + (h >> 2), h - 5, WHITE);
    }
}

void
open_all(Level *l)
{
    for (uint32_t i = 0; i < l->capacity; i++){
        SET_BIT(l->data[i], TILE_OPEN);
    }
    l->closed_tiles = 0;
}

void 
open_tile(Level *l, uint32_t tile_pos, uint8_t by_user)
{
    if (tile_pos >= l->capacity) return;

    // Return if tile is flagged
    if (CHECK_BIT(l->data[tile_pos], FLAGGED)) {
        // TODO: Tile flagging 
        return;
    }
    
    // Return if tile is open allready
    if (CHECK_BIT(l->data[tile_pos], TILE_OPEN)) {
        return;
    }  
    
    // At this point only user can open tile that contains mine.
    if (CHECK_BIT(l->data[tile_pos], IS_MINE) && !by_user) return; //unreachable
    
    // Open all if we hit the mine
    if (CHECK_BIT(l->data[tile_pos], IS_MINE) && by_user) {
        open_all(l);
        return;
    }

    // Now we can open the tile
    div_t pos = div(tile_pos, l->width);
    SET_BIT(l->data[tile_pos], TILE_OPEN);
    l->closed_tiles--;

    // If we have neighborvalue we want to stop recursion
    if (GET_NEIGHBOR_VALUE(l->data[tile_pos]) > 0) {
        return;
    } 


    // recursively open neighbors
#define OPEN_NEXT open_tile(l, next_pos, 0)

    int32_t next_pos;
    // Row above
    if (pos.quot > 0){
        next_pos = (tile_pos - l->width) - 1;
        if (next_pos >= 0) {
            if (pos.rem > 0) OPEN_NEXT;
            next_pos++;
            OPEN_NEXT;
            next_pos++;
            if (pos.rem < l->width - 1) OPEN_NEXT;
        }
    }
    
    // Same row
    next_pos = tile_pos - 1;
    if (pos.rem > 0) open_tile(l, next_pos, 0);
    next_pos = tile_pos + 1;
    if (pos.rem < l->width - 1) open_tile(l, next_pos, 0);

    // Row below
    if (pos.quot < l->height - 1){
        next_pos = (tile_pos + l->width) - 1;
        if ((uint32_t)next_pos < l->capacity){
            if (pos.rem > 0) open_tile(l, next_pos, 0);
            next_pos++;
            open_tile(l, next_pos, 0);
            next_pos++;
            if (pos.rem < l->width) open_tile(l, next_pos, 0);
        }
    }
}

enum GAME_STATES {
    STATE_MENU,
    STATE_GAME,
    STATE_GAMEOVER,
    STATE_WIN
};

int main(void){
    const uint16_t W_WIDTH = 800;
    const uint16_t W_HEIGHT = 800;

    InitWindow(W_WIDTH, W_HEIGHT, "Ephosweeper");
    SetTargetFPS(60);

    srand(time(NULL));

    int x, y;
    
    int level_w = 20;
    int level_h = 20;
    int level_m = 20;

    Level level;
    initialize_level(&level, level_w, level_h, level_m);
    int t_width = W_WIDTH / level_w;
    int t_height = W_HEIGHT / level_h;

    Vector2 mouse;
    int mpx, mpy, hovered = 0;

    uint32_t tile;

    int rpos;

    int gamestate = STATE_GAME;

    int debug = 0;

    while (!WindowShouldClose()){
        BeginDrawing();
        mouse = GetMousePosition();


        // Gameloop
        if (gamestate == STATE_GAME){
            mpx = mouse.x / t_width;
            mpy = mouse.y / t_height;
            for (y = 0; y < level.height; ++y){
                rpos = y * level.width;
                for (x = 0; x < level.width; ++x){
                    tile = rpos + x;
                    hovered = (mpx == x && mpy == y);
                    if (hovered && IsMouseButtonPressed(0)){// && !CHECK_BIT(tile, FLAGGED)){
                        open_tile(&level, tile, 1);
                    }
                    if (hovered && IsMouseButtonPressed(1)){
                        TOGGLE_BIT(level.data[tile], FLAGGED);
                    }
                    show_tile(x * t_width, 
                              y * t_height, 
                              t_width, 
                              t_height,
                              level.data[tile],
                              hovered);
                }
            }
            if (level.mine_amount == level.closed_tiles ) gamestate = STATE_WIN;
            if (level.closed_tiles == 0) gamestate = STATE_GAMEOVER;
        }

        if (gamestate == STATE_WIN){
            DrawText("You win!", 200, 200, 100, BLACK);
            DrawText("Press r to continue", 50, 400, 40, BLACK);
        }

        if (gamestate == STATE_GAMEOVER){
            DrawText("GAMEOVER!", 200, 200, 100, BLACK);
            DrawText("Press r to continue", 50, 400, 40, BLACK);
        }

        if  (debug){
            char dbg[100];
            sprintf(dbg, "closed tiles: %d mine_amount: %d", level.closed_tiles, level.mine_amount);
            DrawText(dbg, 20, 20, 20, GREEN);
        }

        if (IsKeyPressed(KEY_R)){
            free_level(&level);
            initialize_level(&level, level_w, level_h, level_m);
            gamestate = STATE_GAME;
        }

        if (IsKeyPressed(KEY_D)) debug = !debug;


        EndDrawing();
    }
    CloseWindow();
    free(level.data);
    return 0;
}

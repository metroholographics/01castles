#ifndef CASTLES_H
#define CASTLES_H

#include <SDL3/SDL.h>

#define ARRAY_SIZE(a) ((int)((sizeof((a))) / (sizeof((a)[0]))))

#define CLEAR_COLOR  (SDL_Color) {145, 163, 170, 255}
#define LIGHT_SQUARE (SDL_Color) {145, 163, 170, 255}
#define DARK_SQUARE  (SDL_Color) {76, 86, 113, 255}

const int WINDOW_WIDTH  = 900;
const int WINDOW_HEIGHT = 600;
const int BOARD_SIZE    = 560;
const int BOARD_TILE    = 70;
const int FPS           = 30;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *spritesheet;
    SDL_Texture  *board_texture;
} Context;

typedef enum {
    EMPTY=0,
    W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_PAWN,
    B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_PAWN,
    NUM_PIECES
} Piece;

typedef enum {
    A, B, C, D, E, F, G, H
} RANK;

bool       initialise_context(Context *c, const char *title, int width, int height, 
                              const char *spritesheet);
void       destroy_context(Context *c);
void       initialise_default_board(Piece *p);
SDL_FRect* get_piece_sprite_source(Piece p, SDL_FRect *sprite_array);
void       populate_piece_sprite_array(SDL_FRect *sprite_array);


#endif

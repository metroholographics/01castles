#ifndef CASTLES_H
#define CASTLES_H

#include <SDL3/SDL.h>

#define ARRAY_SIZE(a) ((int)((sizeof((a))) / (sizeof((a)[0]))))


const int WINDOW_WIDTH  = 900;
const int WINDOW_HEIGHT = 600; 
const int FPS           = 30;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
} Context;

typedef enum {
    EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING,
} Piece;

typedef enum {
    A, B, C, D, E, F, G, H
} RANK;

bool initialise_context(Context *c, const char *title, int width, int height);
void destroy_context(Context *c);
void initialise_default_board(Piece *p);


#endif
#ifndef CASTLES_H
#define CASTLES_H

#include <SDL3/SDL.h>

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600; 
const int FPS           = 30;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} Context;

bool    initialise_context(Context *c, const char *title, int width, int height);
void    destroy_context(Context *c);

#endif
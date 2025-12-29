#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL_main.h>
#include "castles.h"

const char *TITLE = "01castles";

Context context = {0};

int main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    if (!initialise_context(&context, TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        destroy_context(&context);
        return -1;
    }

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(context.renderer, 50, 100, 50 , 255);
        SDL_RenderClear(context.renderer);

        SDL_RenderPresent(context.renderer);

        SDL_Delay(1000/FPS);
    }

    destroy_context(&context);
    return 0;
}

bool initialise_context(Context *c, const char *title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("!!Error: could not initialise SDL - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~SDL Initialised\n");
    }
    c->window = NULL;
    c->window = SDL_CreateWindow(title, width, height, 0);
    if (!c->window) {
        printf("!!Error: could not create window - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~created window: %dx%d\n", width, height);
    }
    c->renderer = NULL;
    c->renderer = SDL_CreateRenderer(c->window, NULL);
    if (!c->renderer) {
        printf("!!Error: could not create renderer - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~created renderer\n");
    }

    return true;
}


void destroy_context(Context *c) {
    if (c->renderer) {
        SDL_DestroyRenderer(c->renderer);
        printf("**destroyed renderer...\n");
    }
    if (c->window) {
        SDL_DestroyWindow(c->window);
        printf("**destroyed window...\n");
    }
    SDL_Quit();
    printf("**SDL de-initialized\n");
}
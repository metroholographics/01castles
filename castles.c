#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL_main.h>
#include "castles.h"

const char *TITLE = "01castles";
Context context   = {0};
Piece board[8*8];

void
initialise_default_board(Piece *p)
{
    p[A*8+0] = p[A*8+7] = ROOK;
    p[B*8+0] = p[B*8+7] = KNIGHT;
    p[C*8+0] = p[C*8+7] = BISHOP;
    p[D*8+0] = p[D*8+7] = QUEEN;
    p[E*8+0] = p[E*8+7] = KING;
    p[F*8+0] = p[F*8+7] = BISHOP;
    p[G*8+0] = p[G*8+7] = KNIGHT;
    p[H*8+0] = p[H*8+7] = ROOK;

    for (int i = 0; i < 8; i++) {
        p[i*8+1] = PAWN;
        p[i*8+6] = PAWN;
    }
}

int
main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    if (!initialise_context(&context, TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        destroy_context(&context);
        return -1;
    }

    initialise_default_board(board);

    printf("Grid:%d\n", ARRAY_SIZE(board));
    for (int f = 0; f < 8; f++) {
        for (int r = 0; r < 8; r++) {
            printf(".%d.", board[f*8+r]);
        }
        printf("\n");
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

bool
initialise_context(Context *c, const char *title, int width, int height) {
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


void
destroy_context(Context *c) {
    if (c->renderer) {
        SDL_DestroyRenderer(c->renderer);
        printf("**destroyed renderer...\n");
    }

    if (c->window) {
        SDL_DestroyWindow(c->window);
        printf("**destroyed window...\n");
    }

    SDL_Quit();
    printf("**SDL de-initialized...\n");
}
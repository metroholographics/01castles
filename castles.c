#include <stdio.h>
#include <stdbool.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "castles.h"

const char *TITLE       = "01castles";
const char *SPRITESHEET = "assets/spritesheet.png";

Context context = {0};
Piece board[8*8];
SDL_FRect piece_sprite_array[NUM_PIECES];

void
initialise_default_board(Piece *p)
{
    p[A*8+0] = W_ROOK;   p[A*8+7] = B_ROOK;
    p[B*8+0] = W_KNIGHT; p[B*8+7] = B_KNIGHT;
    p[C*8+0] = W_BISHOP; p[C*8+7] = B_BISHOP;
    p[D*8+0] = W_QUEEN;  p[D*8+7] = B_QUEEN;
    p[E*8+0] = W_KING;   p[E*8+7] = B_KING;
    p[F*8+0] = W_BISHOP; p[F*8+7] = B_BISHOP;
    p[G*8+0] = W_KNIGHT; p[G*8+7] = B_KNIGHT;
    p[H*8+0] = W_ROOK;   p[H*8+7] = B_ROOK;

    for (int i = 0; i < 8; i++) {
        p[i*8+1] = W_PAWN;
        p[i*8+6] = B_PAWN;
    }
}

SDL_FRect*
get_piece_sprite_source(Piece p, SDL_FRect *sprite_array)
{
    return &sprite_array[p];
}

int
main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    if (!initialise_context(&context, TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, SPRITESHEET)) {
        destroy_context(&context);
        return -1;
    }

    initialise_default_board(board);
    context.board_texture = SDL_CreateTexture(context.renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, BOARD_SIZE, BOARD_SIZE
    );
    if (!context.board_texture) {
        printf("!!Error: could not create board texture - %s\n", SDL_GetError());
        destroy_context(&context);
        return -1;
    } else {
        printf("~~created board texture: %dx%d\n",
            context.board_texture->w, context.board_texture->h
        );
    }
    populate_piece_sprite_array(piece_sprite_array);

    {
        printf("Grid:%d\n", ARRAY_SIZE(board));
        for (int f = 0; f < 8; f++) {
            for (int r = 0; r < 8; r++) {
                printf(".%d.", board[f*8+r]);
            }
            printf("\n");
        }
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

        SDL_SetRenderTarget(context.renderer, context.board_texture);
        SDL_Color c = CLEAR_COLOR;
        SDL_SetRenderDrawColor(context.renderer, c.r, c.g, c.b, c.a);
        SDL_RenderClear(context.renderer);
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                Piece p = board[row * 8 + col];
                SDL_Color c = (
                    (row + col) % 2 == 0 ? DARK_SQUARE : LIGHT_SQUARE
                );
                SDL_FRect d = (SDL_FRect){col*BOARD_TILE, row*BOARD_TILE, BOARD_TILE, BOARD_TILE};
                SDL_SetRenderDrawColor(context.renderer, c.r, c.g, c.b, c.a);
                SDL_RenderFillRect(context.renderer, &d);
                SDL_RenderTexture(context.renderer, context.spritesheet,
                    get_piece_sprite_source(p, piece_sprite_array),
                    &d
                );
            }
        }

        SDL_SetRenderTarget(context.renderer, NULL);
        c = CLEAR_COLOR;
        SDL_SetRenderDrawColor(context.renderer, c.r, c.g, c.b, c.a);
        SDL_RenderClear(context.renderer);
        SDL_FRect bt = (SDL_FRect) {320, 20, BOARD_SIZE, BOARD_SIZE};
        SDL_RenderTexture(context.renderer, context.board_texture, NULL, &bt);

        SDL_RenderPresent(context.renderer);

        SDL_Delay(1000/FPS);
    }

    destroy_context(&context);
    return 0;
}

bool
initialise_context(Context *c, const char *title, int width, int height, const char* spritesheet)
{
    c->window        = NULL;
    c->renderer      = NULL;
    c->spritesheet   = NULL;
    c->board_texture = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("!!Error: could not initialise SDL - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~SDL Initialised\n");
    }

    c->window = SDL_CreateWindow(title, width, height, 0);
    if (!c->window) {
        printf("!!Error: could not create window - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~created window: %dx%d\n", width, height);
    }

    c->renderer = SDL_CreateRenderer(c->window, NULL);
    if (!c->renderer) {
        printf("!!Error: could not create renderer - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~created renderer\n");
    }

    c->spritesheet = IMG_LoadTexture(c->renderer, spritesheet);
    if (!c->spritesheet) {
        printf("!!Error: could not create spritesheet - %s\n", SDL_GetError());
        return false;
    } else {
        printf("~~loaded and created spritesheet texture\n");
    }

    return true;
}

void
populate_piece_sprite_array(SDL_FRect *sprite_array)
{
    sprite_array[EMPTY]    = (SDL_FRect) {0,    0, 0,          0};
    sprite_array[W_ROOK]   = (SDL_FRect) {0,    0, BOARD_TILE, BOARD_TILE};
    sprite_array[W_KNIGHT] = (SDL_FRect) {70,   0, BOARD_TILE, BOARD_TILE};
    sprite_array[W_BISHOP] = (SDL_FRect) {140,  0, BOARD_TILE, BOARD_TILE};
    sprite_array[W_QUEEN]  = (SDL_FRect) {210,  0, BOARD_TILE, BOARD_TILE};
    sprite_array[W_KING]   = (SDL_FRect) {280,  0, BOARD_TILE, BOARD_TILE};
    sprite_array[W_PAWN]   = (SDL_FRect) {350,  0, BOARD_TILE, BOARD_TILE};
    sprite_array[B_ROOK]   = (SDL_FRect) {0,   70, BOARD_TILE, BOARD_TILE};
    sprite_array[B_KNIGHT] = (SDL_FRect) {70,  70, BOARD_TILE, BOARD_TILE};
    sprite_array[B_BISHOP] = (SDL_FRect) {140, 70, BOARD_TILE, BOARD_TILE};
    sprite_array[B_QUEEN]  = (SDL_FRect) {210, 70, BOARD_TILE, BOARD_TILE};
    sprite_array[B_KING]   = (SDL_FRect) {280, 70, BOARD_TILE, BOARD_TILE};
    sprite_array[B_PAWN]   = (SDL_FRect) {350, 70, BOARD_TILE, BOARD_TILE};
}

void
destroy_context(Context *c)
{
    if (c->board_texture) {
        SDL_DestroyTexture(c->board_texture);
        printf("**destroyed board texture...\n");
    }

    if (c->spritesheet) {
            SDL_DestroyTexture(c->spritesheet);
            printf("**destroyed spritesheet...\n");
        }

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
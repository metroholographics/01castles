#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "castles.h"

#define DEBUG 1

const char *TITLE        = "01castles";
const char *SPRITESHEET  = "assets/spritesheet.png";
const char *PGN_FILEPATH = "example_pgn/1examplepgn.txt";

Context     context                        = {0};
SDL_FRect   piece_sprite_array[NUM_PIECES] = {0};
PGN_Game    pgn_game                       = {0};
TurnHistory turn_history                   = {0};
Piece       *current_board                 = NULL;
int         current_board_index            = 0;

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

    if (pgn_create_game(&pgn_game, PGN_FILEPATH) < 0) {
        printf("!!Error: could not parse provided PGN\n");
        destroy_context(&context);
        return -1;
    } else {
        printf("...Congrats, PGN parsed\n");
    }

    store_game_in_boards(&turn_history, pgn_game);
    current_board = turn_history.game_turns[current_board_index];
    if (DEBUG) {
        printf("Grid:%d\n", ARRAY_SIZE(turn_history.game_turns[current_board_index]));
        for (int f = 0; f < 8; f++) {
            for (int r = 0; r < 8; r++) {
                printf(".%d.", current_board[f*8+r]);
            }
            printf("\n");
        }
    }


    bool running = true;
    SDL_Event e;
    while (running) {
        bool trigger_board_refresh = false;
        SDL_Color r_c;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (e.key.key == SDLK_RIGHT) {
                        trigger_board_refresh = true;
                        current_board_index++;
                    } else if (e.key.key == SDLK_LEFT) {
                        trigger_board_refresh = true;
                        current_board_index--;
                    } else if (e.key.key == SDLK_R) {
                        trigger_board_refresh = true;
                        current_board_index = 0;
                    }
                default:
                    break;
            }
        }

        if (trigger_board_refresh) {
            current_board_index = CLAMP_I(current_board_index, 0, turn_history.num_turns);
            current_board = turn_history.game_turns[current_board_index];
            printf("%d ", current_board_index);
        }
        // Draw to board texture
        SDL_SetRenderTarget(context.renderer, context.board_texture);
        r_c = CLEAR_COLOR;
        SDL_SetRenderDrawColor(context.renderer, r_c.r, r_c.g, r_c.b, r_c.a);
        SDL_RenderClear(context.renderer);
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                Piece p = current_board[row * 8 + col];
                SDL_FRect d = (SDL_FRect){col*BOARD_TILE, row*BOARD_TILE, BOARD_TILE, BOARD_TILE};
                SDL_Color sq_c = (((row + col) % 2 == 0 ) ? DARK_SQUARE : LIGHT_SQUARE);
                // Render board square
                SDL_SetRenderDrawColor(context.renderer, sq_c.r, sq_c.g, sq_c.b, sq_c.a);
                SDL_RenderFillRect(context.renderer, &d);
                // Render piece on board square
                SDL_RenderTexture(
                    context.renderer, context.spritesheet,
                    get_piece_sprite_source(p, piece_sprite_array),
                    &d
                );
            }
        }
        // Draw to window
        SDL_SetRenderTarget(context.renderer, NULL);
        r_c = CLEAR_COLOR;
        SDL_SetRenderDrawColor(context.renderer, r_c.r, r_c.g, r_c.b, r_c.a);
        SDL_RenderClear(context.renderer);
        SDL_FRect board_tex = (SDL_FRect){320, 20, BOARD_SIZE, BOARD_SIZE};
        SDL_RenderTexture(context.renderer, context.board_texture, NULL, &board_tex);

        SDL_RenderPresent(context.renderer);

        SDL_Delay(1000/FPS);
    }

    destroy_context(&context);
    return 0;
}

void
store_game_in_boards(TurnHistory *th, PGN_Game p)
{
    th->num_turns = p.num_turns;
    initialise_default_board(th->game_turns[0]);

    int board_index = 1;
    for (int i = 0; i < th->num_turns; i++) {
        PGN_Turn current_turn = p.move_buffer[i];
        copy_board(th->game_turns[board_index], th->game_turns[board_index-1]);
        if (current_turn.white_move){
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_WHITE);
        }
        board_index++;
        copy_board(th->game_turns[board_index], th->game_turns[board_index-1]);
        if (current_turn.black_move){
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_BLACK);
        }
        board_index++;
        
    }
}

void
input_turn_on_board(Piece* b, PGN_Turn t, int color)
{
    bool castle = t.castle[color];
    bool promotion = t.promotion[color];

    //TODO: handle castle and promotion moves first if exist

    char piece = '\0';
    piece = t.piece[color][0];

    switch (piece) {
        case 'P': handle_pawn_move(b, t.piece[color], t.move_to[color], color); 
    }
}

void
handle_pawn_move(Piece *b, char *piece, char *destination, int color)
{

    //Normal pawn move
    if (piece[1] == '\0') {
        int destination_index = get_index_from_move(destination[0], destination[1]);
        if (destination_index < 0) {
            printf("ERROR DESTINATION INDEX");
            return;
        }

        int sign   = (color == PGN_WHITE) ? -1 : 1;
        Piece pawn = (color == PGN_WHITE) ? W_PAWN : B_PAWN;

        if (b[destination_index + sign] == pawn) {
            b[destination_index] = pawn;
            b[destination_index + sign] = EMPTY;
        } else if (b[destination_index + (sign*2)] == pawn) {
            b[destination_index] = pawn;
            b[destination_index + (sign*2)] = EMPTY;
        } else {
            printf("ERROR: INVALID PAWN MOVE?\n");
            return;
        }
    }

}

int
get_index_from_move(char file, char rank) {
    int f, r;
    switch (file) {
        case 'a': f = 0; break;
        case 'b': f = 1; break;
        case 'c': f = 2; break;
        case 'd': f = 3; break;
        case 'e': f = 4; break;
        case 'f': f = 5; break;
        case 'g': f = 6; break;
        case 'h': f = 7; break;
        default : return -1;
    }
    switch (rank) {
        case '1': r = 0; break;
        case '2': r = 1; break;
        case '3': r = 2; break;
        case '4': r = 3; break;
        case '5': r = 4; break;
        case '6': r = 5; break;
        case '7': r = 6; break;
        case '8': r = 7; break;
        default : return -1;
    }
    return (f * 8) + r;
}


void
copy_board(Piece *target, Piece *source)
{
    for (int i = 0; i < 8*8; i++) {
        target[i] = source[i];
    }
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
        printf("~~SDL initialised\n");
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
        printf("~~loaded and created spritesheet texture: %s\n", spritesheet);
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
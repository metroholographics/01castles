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
            current_board       = turn_history.game_turns[current_board_index];
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
    th->num_turns = p.num_turns*2+2;
    initialise_default_board(th->game_turns[0]);

    int board_index = 1;
    for (int i = 0; i < th->num_turns; i++) {
        PGN_Turn current_turn = p.move_buffer[i];
        copy_board(th->game_turns[board_index], th->game_turns[board_index-1]);
        if (current_turn.white_move) {
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_WHITE);
        }
        board_index++;
        copy_board(th->game_turns[board_index], th->game_turns[board_index-1]);
        if (current_turn.black_move) {
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_BLACK);
        }
        board_index++;
    }
}

void
input_turn_on_board(Piece* b, PGN_Turn t, int color)
{
    bool castle    = t.castle[color];
    bool promotion = t.promotion[color];

    //TODO: handle castle and promotion moves first if exist

    char piece = '\0';
    piece      = t.piece[color][0];

    switch (piece) {
        case 'P': handle_pawn_move(b, t.piece[color], t.move_to[color], color);   break;
        case 'N': handle_knight_move(b, t.piece[color], t.move_to[color], color); break;
        case 'B': handle_bishop_move(b,t.piece[color], t.move_to[color], color);  break;
        case 'R': handle_rook_move(b, t.piece[color], t.move_to[color], color);   break;
    }
}

void
handle_rook_move(Piece *b, char *piece, char *destination, int color)
{
    Piece active_rook       = (color == PGN_WHITE) ? W_ROOK : B_ROOK;
    int   destination_index = get_index_from_move(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: ROOK MOVE\n");
        return;
    }

    if (piece[2] != '\0') {
        int piece_file             = char_to_file_or_rank(piece[1]);
        int piece_rank             = char_to_file_or_rank(piece[2]);
        b[destination_index]       = active_rook;
        b[piece_file*8+piece_rank] = EMPTY;
    } else if (piece[1] != '\0') {
        int file_index = char_to_file_or_rank(piece[1]);
        int rank_index = char_to_file_or_rank(destination[1]);
        int found_rook = hunt_rook(b, file_index, rank_index, active_rook);
        if (found_rook >= 0) {
            b[destination_index] = active_rook;
            b[found_rook]        = EMPTY;
        } else {
            printf("ERROR: INVALID ROOK MOVE - file known\n");
        }
    } else {
        int file_index = char_to_file_or_rank(destination[0]);
        int rank_index = char_to_file_or_rank(destination[1]);
        int found_rook = hunt_rook(b, file_index, rank_index, active_rook);
        if (found_rook >= 0) {
            b[destination_index] = active_rook;
            b[found_rook]        = EMPTY;
        } else {
            printf("ERROR: INVALID ROOK MOVE - normal\n");
        }
    }
}

int
hunt_rook(Piece *b, int file_index, int rank_index, Piece rook)
{
    int max_rooks[8]     = {0};
    int found_rook_count = 0;
    int found_rook_index = -1;
    //first searching the file
    for (int i = file_index*8; i < file_index*8+8; i++) {
        if (b[i] == rook) max_rooks[found_rook_count++] = i;
    }
    if (found_rook_count == 0) {
        //rook not found in file, searching the rank
        for (int i = rank_index; i < 8*8; i+=8) {
            if (b[i] == rook) max_rooks[found_rook_count++] = i;
        }
    }
    if (found_rook_count > 0) {
        int dest_index   = file_index*8+rank_index;
        int lowest_delta = 100;
        for (int i = 0; i < found_rook_count; i++) {
            int delta = ABS_I((dest_index - max_rooks[i]));
            if (delta < lowest_delta) {
                lowest_delta     = delta;
                found_rook_index = max_rooks[i];
            }
        }
    } 
    return found_rook_index;
}

void
handle_bishop_move(Piece *b, char *piece, char *destination, int color)
{
    Piece active_bishop     = (color == PGN_WHITE) ? W_BISHOP : B_BISHOP;
    int   destination_index = get_index_from_move(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: BISHOP MOVE\n");
        return;
    }
    int  file_index       = char_to_file_or_rank(destination[0]);
    int  rank_index       = char_to_file_or_rank(destination[1]);
    bool dest_dark_square = is_dark_square(file_index, rank_index);

    if (piece[2] != '\0') {
        int piece_file             = char_to_file_or_rank(piece[1]);
        int piece_rank             = char_to_file_or_rank(piece[2]);
        b[destination_index]       = active_bishop;
        b[piece_file*8+piece_rank] = 0;
    } else if (piece[1] != '\0') {
        int found_index = -1;
        for (int i = 0; i < 8; i++) {
            int piece_file = char_to_file_or_rank(piece[1]);
            if (b[piece_file*8+i] == active_bishop) {
                if (is_dark_square(piece_file, i) == dest_dark_square) {
                    found_index = piece_file*8+i;
                    break;
                }
            }
        }
        if (found_index >= 0) {
            b[destination_index] = active_bishop;
            b[found_index]       = EMPTY;
        } else {
            printf("ERROR: INVALID BISHOP MOVE: file known\n");
        }
    } else {
        int  found_index = -1;
        for (int f = 0; f < 8; f++) { 
            for (int r = 0; r < 8; r++) {
                if (b[f*8+r] == active_bishop) {
                    if (is_dark_square(f,r) == dest_dark_square) {
                        found_index = f*8+r;
                        break;
                    }
                }
            }
            if (found_index >= 0) break;
        }
        if (found_index >= 0) {
            b[destination_index] = active_bishop;
            b[found_index]       = EMPTY;
        } else {
            printf("ERROR: INVALID BISHOP MOVE - normal\n");
        }
    }
}

void
handle_knight_move(Piece *b, char *piece, char *destination, int color)
{
    Piece active_knight     = (color == PGN_WHITE) ? W_KNIGHT : B_KNIGHT;
    int   destination_index = get_index_from_move(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: KNIGHT MOVE\n");
        return;
    }
    
    if (piece[2] != '\0') {
        int file_index             = char_to_file_or_rank(piece[1]);
        int rank_index             = char_to_file_or_rank(piece[2]);
        b[destination_index]       = active_knight;
        b[file_index*8+rank_index] = EMPTY;
    } else if (piece[1] != '\0') {
        int file_index   = char_to_file_or_rank(piece[1]);
        int found_knight = -1;
        for (int i = file_index*8; i < file_index*8+8; i++) {
            if (b[i] == active_knight) {
                found_knight = i;
                break;
            }
        }
        if (found_knight >= 0) {
            b[destination_index] = active_knight;
            b[found_knight]      = EMPTY;
        } else {
            printf("ERROR: INVALID KNIGHT MOVE? - knight file known\n");
            return;
        }
    } else {
        //we need to find the knight based on destination square
        int file_index    = char_to_file_or_rank(destination[0]);
        int rank_index    = char_to_file_or_rank(destination[1]);
        int moving_knight = hunt_knight(b, file_index, rank_index, active_knight);
        if (moving_knight > 0) {
            b[destination_index] = active_knight;
            b[moving_knight]     = EMPTY;
        } else {
            printf("ERROR: INVALID KNIGHT MOVE? - knight hunt\n");
            return;
        }
    }
}

int
hunt_knight(Piece *b, int file_index, int rank_index, Piece knight)
{
    if (file_index - 2 >= 0) {
        if (rank_index - 1 >= 0) {
            if (b[(file_index-2)*8+(rank_index-1)] == knight) return ((file_index-2)*8+rank_index-1);
        }
        if (rank_index + 1 <= 7) {
            if (b[(file_index-2)*8+(rank_index+1)] == knight) return ((file_index-2)*8+rank_index+1);
        }
    }
    if (file_index - 1 >= 0) {
        if (rank_index - 2 >= 0) {
            if (b[(file_index-1)*8+(rank_index-2)] == knight) return ((file_index-1)*8+rank_index-2);
        }
        if (rank_index + 2 <= 7) {
            if (b[(file_index-1)*8+(rank_index+2)] == knight) return ((file_index-1)*8+rank_index+2);
        }
    }
    if (file_index + 2 <= 7) {
        if (rank_index - 1 >= 0) {
            if (b[(file_index+2)*8+(rank_index-1)] == knight) return ((file_index+2)*8+rank_index-1);
        }
        if (rank_index + 1 <= 7) {
            if (b[(file_index+2)*8+(rank_index+1)] == knight) return ((file_index+2)*8+rank_index+1);
        }
    }
    if (file_index + 1 <= 7) {
        if (rank_index - 2 >= 0) {
            if (b[(file_index+1)*8+(rank_index-2)] == knight) return ((file_index+1)*8+rank_index-2);
        }
        if (rank_index + 2 <= 7) {
            if (b[(file_index+1)*8+(rank_index+2)] == knight) return ((file_index+1)*8+rank_index+2);
        }
    }
    printf("%d%d ", file_index, rank_index);
    return -1;
}

void
handle_pawn_move(Piece *b, char *piece, char *destination, int color)
{
    int   sign              = (color == PGN_WHITE) ? -1 : 1;
    Piece active_pawn       = (color == PGN_WHITE) ? W_PAWN : B_PAWN;
    int   destination_index = get_index_from_move(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: PAWN MOVE\n");
        return;
    }
    //Normal pawn move (no file or rank disambiguation)
    if (piece[1] == '\0') {
        int i                 = destination_index;
        int moving_pawn_index = -1;

        if (b[i+sign*2] == active_pawn) {moving_pawn_index = i + sign*2;}
        if (b[i+sign]   == active_pawn) {moving_pawn_index = i + sign;}

        if (moving_pawn_index >= 0) {
            b[destination_index] = active_pawn;
            b[moving_pawn_index] = EMPTY;
        } else {
            printf("ERROR: INVALID PAWN MOVE? - normal\n");
            return;
        }
    } else {
        if (b[destination_index] != EMPTY) {
            //capture
            int found_index = get_index_from_move(piece[1], destination[1]);
            found_index    += sign;
            if (b[found_index] == active_pawn) {
                b[destination_index] = active_pawn;
                b[found_index]       = EMPTY;
            } else {
                printf("ERROR: INVALID PAWN MOVE? = capture\n");
                return;
            }
        } else {
            //en-passant
            Piece passing_pawn = (active_pawn == W_PAWN) ? B_PAWN : W_PAWN;
            if (b[destination_index-sign] == passing_pawn) {
                b[destination_index]      = active_pawn;
                b[destination_index-sign] = EMPTY;
            } else {
                printf("ERROR: INVALID PAWN MOVE? - en passant\n");
                return;
            }
        }
    }
}

int
char_to_file_or_rank(char c)
{
    switch (c) {
        case 'a': case '1': return 0;
        case 'b': case '2': return 1;
        case 'c': case '3': return 2;
        case 'd': case '4': return 3;
        case 'e': case '5': return 4;
        case 'f': case '6': return 5;
        case 'g': case '7': return 6;
        case 'h': case '8': return 7;
    }
    return -1;
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

bool
is_dark_square(int file, int rank)
{
    return ((file + rank) % 2 == 0);
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
    sprite_array[EMPTY]    = (SDL_FRect) {0,    0,          0,          0};
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

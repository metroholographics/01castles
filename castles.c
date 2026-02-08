#include <stdbool.h>
#include <stdio.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include "castles.h"

#define DEBUG 1

const char *TITLE        = "01castles";
const char *SPRITESHEET  = "assets/spritesheet.png";
const char *PGN_FILEPATH = "example_pgn/3examplepgn.txt";

Context     context                        = {0};
SDL_FRect   piece_sprite_array[NUM_PIECES] = {0};
PGN_Game    pgn_game                       = {0};
TurnHistory turn_history                   = {0};
Piece       *current_board                 = NULL;
int         current_board_index            = 0;

void
initialise_default_board(Piece *p)
{
    p[A * 8 + 0] = W_ROOK;   p[A * 8 + 7] = B_ROOK;
    p[B * 8 + 0] = W_KNIGHT; p[B * 8 + 7] = B_KNIGHT;
    p[C * 8 + 0] = W_BISHOP; p[C * 8 + 7] = B_BISHOP;
    p[D * 8 + 0] = W_QUEEN;  p[D * 8 + 7] = B_QUEEN;
    p[E * 8 + 0] = W_KING;   p[E * 8 + 7] = B_KING;
    p[F * 8 + 0] = W_BISHOP; p[F * 8 + 7] = B_BISHOP;
    p[G * 8 + 0] = W_KNIGHT; p[G * 8 + 7] = B_KNIGHT;
    p[H * 8 + 0] = W_ROOK;   p[H * 8 + 7] = B_ROOK;

    for (int i = 0; i < 8; i++) {
        p[i * 8 + 1] = W_PAWN;
        p[i * 8 + 6] = B_PAWN;
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
         for (int r = 7; r >= 0; r--) {
            for (int f = 0; f < 8; f++) {
                printf("%2d.", current_board[f*8+r]);
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
        for (int c = 7; c >= 0; c--) {
            for (int f = 0; f < 8; f++) {
                Piece p = current_board[f * 8 + c];
                SDL_FRect d = (SDL_FRect){f*BOARD_TILE, (7-c)*BOARD_TILE, BOARD_TILE, BOARD_TILE};
                SDL_Color sq_c = (((f + c) % 2 == 0 ) ? DARK_SQUARE : LIGHT_SQUARE);
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
    th->num_turns = (p.num_turns + 1) * 2;
    initialise_default_board(th->game_turns[0]);

    int board_index = 1;
    for (int i = 0; i < p.num_turns + 1; i++) {
        PGN_Turn current_turn = p.move_buffer[i];
        copy_board(th->game_turns[board_index], th->game_turns[board_index - 1]);
        if (current_turn.white_move) {
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_WHITE);
        }
        board_index++;
        copy_board(th->game_turns[board_index], th->game_turns[board_index - 1]);
        if (current_turn.black_move) {
            input_turn_on_board(th->game_turns[board_index], current_turn, PGN_BLACK);
        }
        board_index++;
    }
    printf("board index: %d\n", board_index);
}

void
input_turn_on_board(Piece* b, PGN_Turn t, int color)
{
    bool castle    = t.castle[color];
    bool promotion = t.promotion[color];

    //TODO: handle castle and promotion moves first if exist
    if (castle) {
        handle_castle(b, t.piece, t.move_to, color);
        return;
    }

    if (promotion) {
        handle_promotion(b, t.piece, t.move_to[color],t.promotion_piece[color], color);
        return;
    }

    char piece = t.piece[color][0];

    switch (piece) {
        case 'P': handle_pawn_move(b, t.piece[color], t.move_to[color], color);   break;
        case 'N': handle_knight_move(b, t.piece[color], t.move_to[color], color); break;
        case 'B': handle_bishop_move(b,t.piece[color], t.move_to[color], color);  break;
        case 'R': handle_rook_move(b, t.piece[color], t.move_to[color], color);   break;
        case 'Q': handle_queen_move(b, t.piece[color], t.move_to[color], color);  break;
        case 'K': handle_king_move(b, t.move_to[color], color);                   break;
    }
}

void
handle_promotion(Piece *b, char (*piece)[4], char *destination, char *prom_piece,int color)
{
    bool valid_input = piece[color][0] == 'P';
    if (!valid_input) {
        printf("ERROR PROMOTION: not a pawn moving?\n");
        return;
    }
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: PROMOTION\n");
        return;
    }

    handle_pawn_move(b, piece[color], destination, color);

    Piece promotion_piece = EMPTY;
    switch (prom_piece[0]) {
        case 'Q': promotion_piece = (color == PGN_WHITE) ? W_QUEEN  : B_QUEEN;  break;
        case 'R': promotion_piece = (color == PGN_WHITE) ? W_ROOK   : B_ROOK;   break;
        case 'B': promotion_piece = (color == PGN_WHITE) ? W_BISHOP : B_BISHOP; break;
        case 'N': promotion_piece = (color == PGN_WHITE) ? W_KNIGHT : B_KNIGHT; break;
    }

    if (promotion_piece != EMPTY) {
        b[destination_index] = promotion_piece;
    } else {
        printf("ERROR PROMOTION: wrong piece?\n");
    }
}

void
handle_castle(Piece *b, char (*piece)[4], char (*destination)[3], int color)
{
    Piece active_king = (color == PGN_WHITE) ? W_KING : B_KING;
    Piece active_rook = (color == PGN_WHITE) ? W_ROOK : B_ROOK;

    bool valid_input = piece[color][0] == 'K' && piece[color + 2][0] == 'R';
    if (!valid_input) {
        printf("ERROR CASTLE: wrong PGN input?\n");
        return;
    }
    bool kingside  = (destination[color][0] == 'g');
    bool queenside = (destination[color][0] == 'c');
    if ((!kingside && !queenside)) {
        printf("ERROR CASTLE: destination\n");
        return;
    }

    int k_dest_index, k_origin_index;
    int r_dest_index, r_origin_index;
    k_dest_index = k_origin_index = -1;
    r_dest_index = r_origin_index = -1;

    if (kingside) {
        k_dest_index   = (color == PGN_WHITE) ? G * 8 + 0 : G * 8 + 7;
        k_origin_index = (color == PGN_WHITE) ? E * 8 + 0 : E * 8 + 7;
        r_dest_index   = (color == PGN_WHITE) ? F * 8 + 0 : F * 8 + 7;
        r_origin_index = (color == PGN_WHITE) ? H * 8 + 0 : H * 8 + 7;
    } else if (queenside) {
        k_dest_index   = (color == PGN_WHITE) ? C * 8 + 0 : C * 8 + 7;
        k_origin_index = (color == PGN_WHITE) ? E * 8 + 0 : E * 8 + 7;
        r_dest_index   = (color == PGN_WHITE) ? D * 8 + 0 : D * 8 + 7;
        r_origin_index = (color == PGN_WHITE) ? A * 8 + 0 : A * 8 + 7;
    }

    if (b[k_origin_index] != active_king) {
        printf("ERROR CASTLE: no king\n");
        return;
    }
    if (b[r_origin_index] != active_rook) {
        printf("ERROR CASTLE: no rook\n");
        return;
    }
    move_piece(b, k_origin_index, k_dest_index);
    move_piece(b, r_origin_index, r_dest_index);
    return;
}

void
handle_king_move(Piece *b, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: KING MOVE\n");
        return;
    }
    Piece active_king = (color == PGN_WHITE) ? W_KING : B_KING;
    int   found_king  = -1;

    for (int i = 0; i < 64; i++) {
        if (b[i] == active_king) {
            found_king = i;
            break;
        }
    }

    if (found_king >= 0) {
        move_piece(b, found_king, destination_index);
    } else {
        printf("ERROR KING MOVE\n");
    }
}

void
handle_queen_move(Piece *b, char *piece, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: QUEEN MOVE\n");
        return;
    }
    Piece active_queen = (color == PGN_WHITE) ? W_QUEEN : B_QUEEN;
    int   found_queen  = -1;

    if (piece[2] != '\0') {
        found_queen = get_index_from_square(piece[1], piece[2]);
    } else if (piece[1] != '\0') {
        bool file_known = is_file(piece[1]);
        bool rank_known = is_rank(piece[1]);
        int index       = char_to_file_or_rank(piece[1]);
        if (file_known) {
            for (int i = index * 8; i < index * 8 + 8; i++) {
                if (b[i] == active_queen && not_pinned(b, i, destination_index, color)) {
                    if (trace_clear_line(b, i, destination_index, ANY)) {
                        found_queen = i;
                        break;
                    }
                }
            }
        } else if (rank_known) {
            for (int i = index; i < 64; i += 8) {
                if (b[i] == active_queen && not_pinned(b, i, destination_index, color)) {
                    if (trace_clear_line(b, i, destination_index, ANY)) {
                        found_queen = i;
                        break;
                    }
                }
            }
        }
        if (found_queen < 0) {
            printf("ERROR: INVALID QUEEN MOVE - file/rank known\n");
        }
    } else {
        for (int i = 0; i < 64; i++) {
            if (b[i] == active_queen && not_pinned(b, i, destination_index, color)) {
                if (trace_clear_line(b, i, destination_index, ANY)) {
                    found_queen = i;
                    break;
                }
            }
        }
        if (found_queen < 0) {
            printf("ERROR: INVALID QUEEN MOVE - normal\n");
        }
    }

    if (found_queen >= 0) move_piece(b, found_queen, destination_index);

}

void
handle_rook_move(Piece *b, char *piece, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: ROOK MOVE\n");
        return;
    }
    Piece active_rook = (color == PGN_WHITE) ? W_ROOK : B_ROOK;
    int   found_rook  = -1;

    if (piece[2] != '\0') {
        found_rook = get_index_from_square(piece[1], piece[2]);
    } else if (piece[1] != '\0') {
        bool file_known = is_file(piece[1]);
        bool rank_known = is_rank(piece[1]);
        int index       = char_to_file_or_rank(piece[1]);
        if (file_known) {
            for (int i = index * 8; i < index * 8 + 8; i++) {
                if (b[i] == active_rook && not_pinned(b, i, destination_index, color)) {
                    if (trace_clear_line(b, i, destination_index, STRAIGHT)) {
                        found_rook = i;
                        break;
                    }
                }
            }
        } else if (rank_known) {
            for (int i = index; i < 64; i += 8) {
                if (b[i] == active_rook && not_pinned(b, i, destination_index, color)) {
                    if (trace_clear_line(b, i, destination_index, STRAIGHT)) {
                        found_rook = i;
                        break;
                    }
                }
            }
        }
        if (found_rook < 0) {
            printf("ERROR: INVALID ROOK MOVE - file/rank known\n");
        }
    } else {
        for (int i = 0; i < 64; i++) {
            if (b[i] == active_rook && not_pinned(b, i, destination_index, color)) {
                if (trace_clear_line(b, i, destination_index, STRAIGHT)) {
                    found_rook = i;
                    break;
                }
            }
        }
        if (found_rook < 0) {
            printf("ERROR: INVALID ROOK MOVE - normal\n");
        }
    }

    if (found_rook >= 0) move_piece(b, found_rook, destination_index);
}

void
handle_bishop_move(Piece *b, char *piece, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: BISHOP MOVE\n");
        return;
    }
    Piece active_bishop    = (color == PGN_WHITE) ? W_BISHOP : B_BISHOP;
    int   found_bishop     = -1;
    bool  dest_dark_square = is_dark_square(destination_index);

    if (piece[2] != '\0') {
        found_bishop   = get_index_from_square(piece[1], piece[2]);
    } else if (piece[1] != '\0') {
        bool file_known = is_file(piece[1]);
        bool rank_known = is_rank(piece[1]);
        int index       = char_to_file_or_rank(piece[1]);
        if (file_known) {
            for (int i = index * 8; i < index * 8 + 8; i++) {
                if (b[i] == active_bishop) {
                    if (is_dark_square(i) == dest_dark_square && not_pinned(b, i, destination_index, color)) {
                        if (trace_clear_line(b, i, destination_index, DIAGONAL)) {
                            found_bishop = i;
                            break;
                        }
                    }
                }
            }
        } else if (rank_known) {
            for (int i = index; i < 64; i += 8) {
                if (b[i] == active_bishop) {
                    if (is_dark_square(i) == dest_dark_square && not_pinned(b, i, destination_index, color)) {
                        if (trace_clear_line(b, i, destination_index, DIAGONAL)) {
                            found_bishop = i;
                            break;
                        }
                    }
                }
            }
        }
        if (found_bishop < 0) {
            printf("ERROR: INVALID BISHOP MOVE: file/rank known\n");
            return;
        }
    } else {
        for (int i = 0; i < 64; i++) {
            if (b[i] == active_bishop) {
                if (is_dark_square(i) == dest_dark_square && not_pinned(b, i, destination_index, color)) {
                    if (trace_clear_line(b, i, destination_index, DIAGONAL)) {
                        found_bishop = i;
                        break;
                    }
                }
            }
        }
        if (found_bishop < 0) {
            printf("ERROR: INVALID BISHOP MOVE - normal\n");
            return;
        }
    }

    if (found_bishop >= 0) move_piece(b, found_bishop, destination_index);
}

void
handle_knight_move(Piece *b, char *piece, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: KNIGHT MOVE\n");
        return;
    }
    Piece active_knight = (color == PGN_WHITE) ? W_KNIGHT : B_KNIGHT;
    int   found_knight  = -1;

    if (piece[2] != '\0') {
        found_knight = get_index_from_square(piece[1], piece[2]);
    } else if (piece[1] != '\0') {
        bool file_known = is_file(piece[1]);
        bool rank_known = is_rank(piece[1]);
        int  index      = char_to_file_or_rank(piece[1]);
        if (file_known) {
            for (int i = index * 8; i < index * 8 + 8; i++) {
                if (b[i] == active_knight && not_pinned(b, i, destination_index, color)) {
                    if (validate_knight_move(i, destination_index)) {
                        found_knight = i;
                        break;
                    }
                }
            }
        } else if (rank_known) {
            for (int i = index; i < 64; i += 8) {
                if (b[i] == active_knight && not_pinned(b, i, destination_index, color)) {
                    if (validate_knight_move(i, destination_index)) {
                        found_knight = i;
                        break;
                    }
                }
            }
        }
        if (found_knight < 0) {
            printf("ERROR: INVALID KNIGHT MOVE? - knight file/rank known\n");
            return;
        }
    } else {
        for (int i = 0; i < 64; i++) {
            if (b[i] == active_knight && not_pinned(b, i, destination_index, color)) {
                if (validate_knight_move(i, destination_index)) {
                    found_knight = i;
                    break;
                }
            }
        }
        if (found_knight < 0) {
            printf("ERROR: INVALID KNIGHT MOVE? - knight hunt\n");
            return;
        }
    }

    if (found_knight >= 0) move_piece(b, found_knight, destination_index);
}

bool
validate_knight_move(int origin_index, int destination_index)
{
    int o_f = origin_index / 8;
    int o_r = origin_index % 8;
    int d_f = destination_index / 8;
    int d_r = destination_index % 8;
    return (
        (o_f == (d_f - 2) && o_r == (d_r - 1)) ||
        (o_f == (d_f - 2) && o_r == (d_r + 1)) ||
        (o_f == (d_f - 1) && o_r == (d_r - 2)) ||
        (o_f == (d_f - 1) && o_r == (d_r + 2)) ||
        (o_f == (d_f + 1) && o_r == (d_r - 2)) ||
        (o_f == (d_f + 1) && o_r == (d_r + 2)) ||
        (o_f == (d_f + 2) && o_r == (d_r - 1)) ||
        (o_f == (d_f + 2) && o_r == (d_r + 1))
    );
}

void
handle_pawn_move(Piece *b, char *piece, char *destination, int color)
{
    int destination_index = get_index_from_square(destination[0], destination[1]);
    if (destination_index < 0) {
        printf("ERROR DESTINATION INDEX: PAWN MOVE\n");
        return;
    }

    Piece active_pawn = (color == PGN_WHITE) ? W_PAWN : B_PAWN;
    int   sign        = (color == PGN_WHITE) ? -1 : 1;
    int   found_pawn  = -1;
    //Normal pawn move (no file or rank disambiguation)
    if (piece[1] == '\0') {
        if (b[destination_index + 2*sign] == active_pawn) found_pawn = destination_index + 2*sign;
        if (b[destination_index + sign]   == active_pawn) found_pawn = destination_index + sign;
        if (found_pawn < 0) {
            printf("ERROR: INVALID PAWN MOVE? - normal\n");
            return;
        }
    } else {
        int file = char_to_file_or_rank(piece[1]);
        for (int i = file * 8; i < file * 8 + 8; i++) {
            if (b[i] == active_pawn) {
                if (trace_clear_line(b, i , destination_index, DIAGONAL)) {
                    found_pawn = i;
                    break;
                }
            }
        }

        if (found_pawn < 0) {
            printf("ERROR: INVALID PAWN MOVE? = capture\n");
            return;
        }

        if (b[destination_index] == EMPTY) {
            //en-passant
            Piece passing_pawn = (active_pawn == W_PAWN) ? B_PAWN : W_PAWN;
            if (b[destination_index + sign] == passing_pawn) {
                b[destination_index + sign] = EMPTY;
            } else {
                printf("ERROR: INVALID PAWN MOVE? - en passant\n");
                return;
            }
        }
    }

    if (found_pawn >= 0) move_piece(b, found_pawn, destination_index);
}

void
move_piece(Piece* b, int from_index, int dest_index)
{
    b[dest_index] = b[from_index];
    b[from_index] = EMPTY;
}

bool
not_pinned(Piece *b, int piece_index, int dest_index, int color)
{
    // return true if piece is not pinned
    // piece is pinned if it has an uninterrupted diagonal or straight ('Dir') to it's king
    // && an uninterrupted 'Dir' to Rook or Queen (Dir==straight) or Bishop, Queen (Dir==diag.)
    Piece king       = (color == PGN_WHITE) ? W_KING : B_KING;
    int   found_king = -1;
    for (int i = 0; i < 64; i++) {
        if (b[i] == king) {
            found_king = i;
        }
    }
    if (found_king < 0) {
        printf("ERROR: can't find king in not_pinned()\n");
        return false;
    }

    PathVec k_to_piece = get_path_vector(found_king, piece_index);
    int k_file = found_king / 8;
    int k_rank = found_king % 8;

    if (trace_clear_line(b, piece_index, found_king, STRAIGHT)) {
        Piece opp_rook  = (color == PGN_WHITE) ? B_ROOK : W_ROOK;
        Piece opp_queen = (color == PGN_WHITE) ? B_QUEEN : W_QUEEN;
        while (piece_index >= 0 && piece_index < 64) {
            piece_index += 8 * k_to_piece.f_step;
            piece_index += k_to_piece.r_step;
            if (piece_index == dest_index) return true;
            Piece p = b[piece_index];
            if (p != EMPTY) {
                if (p == opp_rook || p == opp_queen) {
                    bool same_file = (piece_index / 8) == k_file;
                    bool same_rank = (piece_index % 8) == k_rank;
                    if (same_file && (dest_index / 8 != k_file)) {
                        return false;
                    } else if (same_rank && (dest_index % 8 != k_rank)) {
                        return false;
                    } else {
                        return true;
                    }
                }
                else return true;
            }
        }
    } else if (trace_clear_line(b, piece_index, found_king, DIAGONAL)) {
        Piece opp_bishop = (color == PGN_WHITE) ? B_BISHOP : W_BISHOP;
        Piece opp_queen  = (color == PGN_WHITE) ? B_QUEEN : W_QUEEN;
        while (piece_index >= 0 && piece_index < 64) {
            piece_index += 8 * k_to_piece.f_step;
            piece_index += k_to_piece.r_step;
            if (piece_index == dest_index) return true;
            Piece p = b[piece_index];
            if (p != EMPTY) {
                if (p == opp_bishop || p == opp_queen) {
                    PathVec k_to_dest = get_path_vector(found_king, dest_index);
                    return (
                        (k_to_dest.f_step == k_to_piece.f_step) && 
                        (k_to_dest.r_step == k_to_piece.r_step)
                    );
                }
                else return true;
            }
        }
    }
    return true;
}

PathVec
get_path_vector(int from_index, int to_index)
{
    int from_file = from_index / 8;
    int from_rank = from_index % 8;
    int to_file   = to_index / 8;
    int to_rank   = to_index % 8;
    int v_f       = to_file - from_file;
    int v_r       = to_rank - from_rank;
    int sign_f    = ((v_f == 0) ? 0 : v_f/ABS_I(v_f));
    int sign_r    = ((v_r == 0) ? 0 : v_r/ABS_I(v_r));
    return (PathVec) {
        .f_step = sign_f,
        .r_step = sign_r,
    };
}

bool
trace_clear_line(Piece *b, int origin_index, int destination_index, LineType line)
{
    PathVec p_vec = get_path_vector(origin_index, destination_index);

    if (line == STRAIGHT) {
        if (p_vec.f_step != 0 && p_vec.r_step != 0) return false;
    } else if (line == DIAGONAL) {
        if (p_vec.f_step == 0 || p_vec.r_step == 0) return false;
    }

    while (origin_index != destination_index) {
        origin_index += 8 * p_vec.f_step;
        origin_index += p_vec.r_step;
        if (origin_index == destination_index) break;
        if (origin_index < 0 || origin_index >= 64) break;
        if (b[origin_index] != EMPTY) return false;
    }
    return (origin_index == destination_index);
}

bool
is_file(char c) {
    return (c >= 'a' && c <= 'h');
}

bool
is_rank(char c) {
    return (c >= '1' && c <= '8');
}

bool
is_dark_square(int index)
{
    int file = index / 8;
    int rank = index % 8;
    return ((file + rank) % 2 == 0);
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
get_index_from_square(char file, char rank) {
    int f = char_to_file_or_rank(file);
    int r = char_to_file_or_rank(rank);
    return (f * 8) + r;
}

void
copy_board(Piece *target, Piece *source)
{
    for (int i = 0; i < 64; i++) {
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

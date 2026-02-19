#ifndef CASTLES_H
#define CASTLES_H

#include <SDL3/SDL.h>
#include "pgn_reader.h"

#define ARRAY_SIZE(a)      ((int)((sizeof((a))) / (sizeof((a)[0]))))
#define CLAMP_I(x,min,max) ((int)(((x) < (min)) ? (min) : ((x) > (max)) ? (max) : (x)))
#define ABS_I(x)           ((int)((x) < 0 ? (-x) : (x)))

#define CLEAR_COLOR  (SDL_Color) {145, 163, 170, 255}
#define LIGHT_SQUARE (SDL_Color) {145, 163, 170, 255}
#define DARK_SQUARE  (SDL_Color) {76, 86, 113, 255}

typedef enum {
    CSTL_BAD_STORE   = -28,
    CSTL_INVLD_PIECE = -27,
    CSTL_K_NORMAL    = -26,
    CSTL_K_DEST      = -25,
    CSTL_Q_NORMAL    = -24,
    CSTL_Q_KNOWN     = -23,
    CSTL_Q_DEST      = -22,
    CSTL_R_NORMAL    = -21,
    CSTL_R_KNOWN     = -20,
    CSTL_R_DEST      = -19,
    CSTL_B_NORMAL    = -18,
    CSTL_B_KNOWN     = -17,
    CSTL_B_DEST      = -16,
    CSTL_N_NORMAL    = -15,
    CSTL_N_KNOWN     = -14,
    CSTL_N_DEST      = -13,
    CSTL_P_EN_PASS   = -12,
    CSTL_P_CAPTURE   = -11,
    CSTL_P_NORMAL    = -10,
    CSTL_P_DEST      = -9,
    CSTL_CSTL_ROOK   = -8,
    CSTL_CSTL_KING   = -7,
    CSTL_CSTL_DEST   = -6,
    CSTL_CSTL_PIECE  = -5,
    CSTL_PRM_PMOVE   = -4,
    CSTL_PRM_PSQUARE = -3,
    CSTL_PRM_DEST    = -2,
    CSTL_PRM_NO_PAWN = -1,
    CSTL_SUCCESS = 0
} CSTL_Error;

typedef enum {
    EMPTY=0,
    W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_PAWN,
    B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_PAWN,
    NUM_PIECES
} Piece;

typedef enum {
    A, B, C, D, E, F, G, H
} RANK;

typedef enum {
    ANY,
    STRAIGHT,
    DIAGONAL,
} LineType;

typedef struct {
    int f_step;
    int r_step;
} PathVec;

typedef struct {
    Piece game_turns[PGN_MAX_TURNS*2][8*8];
    int   num_turns;
} TurnHistory;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *spritesheet;
    SDL_Texture  *board_texture;
} Context;


bool initialise_context(Context *c, const char *title, int width, int height,
    const char *spritesheet);
void destroy_context(Context *c);

SDL_FRect* get_piece_sprite_source(Piece p, SDL_FRect *sprite_array);
void       populate_piece_sprite_array(SDL_FRect *sprite_array);

void       initialise_default_board(Piece *p);
void       copy_board(Piece *target, Piece *source);
CSTL_Error store_game_in_boards(TurnHistory *th, PGN_Game p);
CSTL_Error input_turn_on_board(Piece* b, PGN_Turn t, int color);

bool    is_file(char c);
bool    is_rank(char c);
int     char_to_file_or_rank(char c);
int     get_index_from_square(char file, char rank);
bool    is_dark_square(int index);
bool    trace_clear_line(Piece *b, int origin_index, int destination_index, LineType line);
bool    not_pinned(Piece *b, int piece_index, int dest_index, int color);
PathVec get_path_vector(int from_index, int to_index);
void    move_piece(Piece *b, int from_index, int dest_index);

CSTL_Error cstl_log(CSTL_Error e);
CSTL_Error handle_castle(Piece *b, char (*piece)[4], char (*destination)[3], int color);
CSTL_Error handle_promotion(Piece *b, char (*piece)[4], char *destination, char *prom_piece,int color);
CSTL_Error handle_pawn_move(Piece *b, char *piece, char *destination, int color);
CSTL_Error handle_knight_move(Piece *b, char *piece, char *destination, int color);
bool       validate_knight_move(int origin_index, int destination_index);
CSTL_Error handle_bishop_move(Piece *b, char *piece, char *destination, int color);
CSTL_Error handle_rook_move(Piece *b, char *piece, char *destination, int color);
CSTL_Error handle_queen_move(Piece *b, char *piece, char *destination, int color);
CSTL_Error handle_king_move(Piece *b, char *destination, int color);

#endif

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

const int WINDOW_WIDTH  = 900;
const int WINDOW_HEIGHT = 600;
const int BOARD_SIZE    = 560;
const int BOARD_TILE    = BOARD_SIZE / 8;
const int FPS           = 30;

typedef enum {
    EMPTY=0,
    W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_PAWN,
    B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_PAWN,
    NUM_PIECES
} Piece;

typedef enum {
    A, B, C, D, E, F, G, H
} RANK;

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

void initialise_default_board(Piece *p);
void store_game_in_boards(TurnHistory *th, PGN_Game p);
void copy_board(Piece *target, Piece *source);
void input_turn_on_board(Piece* b, PGN_Turn t, int color);

int  char_to_file_or_rank(char c);
int  get_index_from_move(char file, char rank);
bool is_dark_square(int file, int rank);


void handle_pawn_move(Piece *b, char *piece, char *destination, int color);
void handle_knight_move(Piece *b, char *piece, char *destination, int color);
int  hunt_knight(Piece *b, int file_index, int rank_index, Piece knight);
void handle_bishop_move(Piece *b, char *piece, char *destination, int color);
void handle_rook_move(Piece *b, char *piece, char *destination, int color);
int  hunt_rook(Piece *b, int file_index, int rank_index, Piece rook);
void handle_queen_move(Piece *b, char *piece, char *destination, int color);
bool validate_queen_move(Piece *b, int origin_index, int destination_index);
void handle_king_move(Piece *b, char *destination, int color);
void handle_castle(Piece *b, char (*piece)[4], char (*destination)[3], int color);
void handle_promotion(Piece *b, char (*piece)[4], char *destination, char *prom_piece,int color);

#endif

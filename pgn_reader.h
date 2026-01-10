#ifndef PGN_READER_H
#define PGN_READER_H


#define PGN_LOG_ERROR(e) (printf("!!PGN_Error: %s\n", pgn_get_error((e))))

#define PGN_MAX_TURNS 300

#define PGN_WHITE 0
#define PGN_BLACK 1
#define PGN_ROOK_OFFSET 2

typedef enum {
    PGN_ERR_SCANLINE  = -6,
    PGN_ERR_ENDGAME   = -5,
    PGN_ERR_PARSE_MTX = -4,
    PGN_ERR_STRIP_TAG = -3,
    PGN_ERR_TAG_BRACE = -2,
    PGN_ERR_FILE_OPEN = -1,
    PGN_SUCCESS = 0,
} PGN_Error;

typedef struct {
    char piece[4][4];
    char move_to[4][3];
    char promotion_piece[2][2];
    bool castle[2];
    bool promotion[2];
    bool white_move;
    bool black_move;
} PGN_Turn;

typedef struct {
    int      num_turns;
    PGN_Turn move_buffer[PGN_MAX_TURNS];
} PGN_Game;

const char* pgn_get_error(PGN_Error e);
PGN_Error pgn_create_game(PGN_Game *g, const char *filepath);
PGN_Error strip_tag_pairs(FILE *f);
PGN_Error parse_movetext(PGN_Game *g, FILE *f);
PGN_Error pgn_read_turn(PGN_Turn *t, FILE *f);
bool file_check_nextc(FILE *f, char c);
int pgn_read_move(char* buff, int buff_max, FILE *f);
void pgn_populate_game_turn(PGN_Turn *t, char *buffer, int len, int color_index);
bool pgn_is_piece(char c);
bool pgn_is_rank(char c);
bool pgn_piece_or_rank(char c);


#endif

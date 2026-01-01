#ifndef PGN_READER_H
#define PGN_READER_H

#include "castles.h"

#define PGN_LOG_ERROR(e) (printf("!!PGN_Error: %s\n", pgn_get_error((e))))

#define MAX_TURNS 300



typedef enum {
    PGN_ERR_PARSE_MTX = -4,
    PGN_ERR_STRIP_TAG = -3,
    PGN_ERR_TAG_BRACE = -2,
    PGN_ERR_FILE_OPEN = -1,
    PGN_SUCCESS = 0,
} PGN_Error;


typedef struct {
    char *piece[2][3];
    char *move_to[2][3];
} PGN_Turn;

typedef struct {
    int      num_turns;
    PGN_Turn move_buffer[MAX_TURNS];
} PGN_Game;

const char* pgn_get_error(PGN_Error e);
PGN_Error pgn_create_game(PGN_Game *g, const char *filepath);
PGN_Error strip_tag_pairs(FILE *f);
PGN_Error parse_movetext(PGN_Game *g, FILE *f);



#endif

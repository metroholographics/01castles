#include <stdio.h>
#include <ctype.h>
#include "pgn_reader.h"

int
main(void)
{
    PGN_Game g = {0};
    if (pgn_create_game(&g, "example_pgn/3examplepgn.txt") < 0) {
        printf("quitting\n");
        return -1;
    }

    printf("\nCongrats - PGN parsed\n");
    return 0;
}

PGN_Error
pgn_create_game(PGN_Game *g, const char *filepath)
{
    //::TODO::ensure valid file format? e.g. only .txt or .pgn?
    FILE *pgn_file = fopen(filepath, "rb");
    if (!pgn_file) {
        PGN_LOG_ERROR(PGN_ERR_FILE_OPEN);
        return PGN_ERR_FILE_OPEN;
    } else {
        printf("...loaded in pgn %s\n", filepath);
    }

    if (strip_tag_pairs(pgn_file) < 0) {
        fclose(pgn_file);
        PGN_LOG_ERROR(PGN_ERR_STRIP_TAG);
        return PGN_ERR_STRIP_TAG;
    } else {
        printf("...stripped tag pairs from pgn data\n");
    }

    if (parse_movetext(g, pgn_file) < 0) {
        fclose(pgn_file);
        PGN_LOG_ERROR(PGN_ERR_PARSE_MTX);
        return PGN_ERR_PARSE_MTX;
    }

    fclose(pgn_file);
    return PGN_SUCCESS;
}

PGN_Error
parse_movetext(PGN_Game *g, FILE *f)
{
    fpos_t start;
    fgetpos(f, &start);
    
    //REMOVE AFTER TESTING
    {   
        char c = '\0';
        while ((c = getc(f)) != EOF)
        printf("%c", c);
        printf("\n\n");
        fsetpos(f, &start);
    }
    
    g->num_turns = -1;
    for (;;) {
        int i = ++g->num_turns;
        printf("Reading turn %d\n", i+1);
        if (pgn_read_turn(&g->move_buffer[i], f) < 0) {
            printf("\nGame ended\n");
            if (!g->move_buffer[i].white_move && !g->move_buffer[i].black_move)
                g->num_turns--;
            break;
        };
    }

    printf("Move buffer has %d turns (index 0)\n", g->num_turns);
    return PGN_SUCCESS;
}

void
pgn_populate_game_turn(PGN_Turn *t, char *buffer, int len, int color_index)
{
    t->castle[color_index] = false;
    t->promotion[color_index] = false;
    int start = 0;
    int end = len;
    while (!pgn_piece_or_rank(buffer[start])) {
        start++;
        len--;
    }

    char c = buffer[end-1];
    if (c == '#' || c == '+') {
        end -= 1;
        len -= 1;
    }

    char castle_check    = buffer[end-1];
    char promotion_check = buffer[end-2];
    if (castle_check == 'O') {
        //castle
        t->castle[color_index]       = true;
        t->piece[color_index][0]     = 'K';
        t->piece[color_index][1]     = '\0';
        t->piece[color_index + 2][0] = 'R';
        t->piece[color_index + 2][1] = '\0';
        //filling rank for kingside(len3 "O-O") vs queenside(len5 - "O-O-O") castle
        if (len == 3) {
            t->move_to[color_index][0]     = 'g';
            t->move_to[color_index][2]     = '\0';
            t->move_to[color_index + 2][0] = 'f';
            t->move_to[color_index + 2][2] = '\0';
        } else if (len == 5) {
            t->move_to[color_index][0]     = 'c';
            t->move_to[color_index][2]     = '\0';
            t->move_to[color_index + 2][0] = 'd';
            t->move_to[color_index + 2][2] = '\0';
        }
        //filling file for white vs black castle e.g. [K] to [g][1/8]
        t->move_to[color_index][1]     = (color_index == PGN_WHITE) ? '1' : '8';
        t->move_to[color_index + 2][1] = (color_index == PGN_WHITE) ? '1' : '8';
    } else if (promotion_check == '=') {
        //promotion
        t->promotion[color_index] = true;
        t->piece[color_index][0] = 'P';
        t->piece[color_index][1] = buffer[0];
        t->piece[color_index][2] = '\0';
        t->move_to[color_index][0] = buffer[end-4];
        t->move_to[color_index][1] = buffer[end-3];
        t->move_to[color_index][2] = '\0';
        t->promotion_piece[color_index][0] = buffer[end-1];
        t->promotion_piece[color_index][1] = '\0';
    } else {
        t->move_to[color_index][0] = buffer[end-2];
        t->move_to[color_index][1] = buffer[end-1];
        t->move_to[color_index][2] = '\0';

        len -= 2;
        if (len == 0) {
            t->piece[color_index][0] = 'P';
            t->piece[color_index][1] = '\0';
        } else if (len == 1) {
            t->piece[color_index][0] = buffer[0];
            t->piece[color_index][1] = '\0';
        } else if (len >= 2) {
            if (buffer[start+1] == 'x') {
                if (pgn_is_piece(buffer[start])) {
                    t->piece[color_index][0] = buffer[start];
                    t->piece[color_index][1] = '\0';
                } else {
                    t->piece[color_index][0] = 'P';
                    t->piece[color_index][1] = buffer[start];
                    t->piece[color_index][2] = '\0';
                }
            } else {
                int i, j;
                for (i = start, j=0; i < start+3; i++, j++) {
                    if (buffer[i] == 'x' || i >= len) {
                        break;
                    }
                    t->piece[color_index][j] = buffer[i];
                }
                if (i < start+3) {
                    t->piece[color_index][2] = '\0';
                } else {
                    t->piece[color_index][3] = '\0';
                }
            }
        }
    }

    printf("\n---Piece %s moves to %s\n", t->piece[color_index], t->move_to[color_index]);
    if (t->castle[color_index]) {
        printf("\t %s moves to %s\n", t->piece[color_index+2], t->move_to[color_index+2]);
    }
    if (t->promotion[color_index]) {
        printf("\t %s promotes to %s\n", t->piece[color_index], t->promotion_piece[color_index]);
    }
}

PGN_Error
pgn_read_turn(PGN_Turn *t, FILE *f)
{
    bool white_done, black_done;
    white_done = black_done = false;
    char white_move_buffer[9], black_move_buffer[18];
    int white_len, black_len, dummy;
    white_len = black_len = dummy = 0;
    t->white_move = t->black_move = false;
    char c = '\0';

    //read and discard turn number
    if ((fscanf(f, "%d", &dummy)) == 0) {
        PGN_LOG_ERROR(PGN_ERR_SCANLINE);
        return PGN_ERR_ENDGAME;
    }; 
    printf("##%d: ", dummy);
    //read and discard '.'
    getc(f);

    if (file_check_nextc(f, '.')) {
        int i = 0;
        while ((c = getc(f)) != ' ') {
            white_move_buffer[i++] = c;
        } //c = ' '
        white_len = i;
        white_done = true;
    }

    while((c = getc(f)) == ' ' || c == '\r' || c == '\n');
    ungetc(c, f);

    if (!white_done) {
        white_len = pgn_read_move(white_move_buffer, 9, f);
        if (white_move_buffer[white_len-1] == '#') {
            //parse move: checkmate
            return PGN_ERR_ENDGAME;
        }
        if (white_len <= 0) {
            return PGN_ERR_ENDGAME;
        } else {
            pgn_populate_game_turn(t, white_move_buffer, white_len, PGN_WHITE);
            t->white_move = true;
        }
    }
    for (int i = 0; i < white_len; i++) {
        char j = white_move_buffer[i];
        printf("%c", j);
    }

    if (c == ' ') {
        while((c = getc(f)) == ' ');
        ungetc(c, f);
    }
    if (file_check_nextc(f, '$')) {
        c = getc(f);
        while((c = getc(f)) != ' ');
    }

    black_len = pgn_read_move(black_move_buffer, 18, f);
    if (black_len <= 0) {
        return PGN_ERR_ENDGAME;
    } else {
        pgn_populate_game_turn(t, black_move_buffer, black_len, PGN_BLACK);
        t->black_move = true;
    }

    if (file_check_nextc(f, '$')) {
        while((c = getc(f)) != ' ');
    }
    if (file_check_nextc(f, '{')) {
        c = getc(f);
        while ((c = getc(f)) != '}');
    }
    if (file_check_nextc(f, ';')) {
        c = getc(f);
        while ((c = getc(f)) != '\n');
        //ungetc(c,f);
    }

    for (int i = 0; i < black_len; i++) {
        char j = black_move_buffer[i];
        printf("%c", j);
    }

    printf("\n");
    return PGN_SUCCESS;
}

int
pgn_read_move(char* buff, int buff_max, FILE* f)
{
    //TODO: Check for end-move (1-0, 0-1, or 1/2-1/2 instead of EOF?)
    char c = '\0';
    int i = 0;
    int len = 0;

    while ((c = getc(f)) != ' ' && c != '\r') {
        if (len == buff_max) {
            printf("BUFFER ERROR");
            return len;
        }
        if (c == EOF) {
            return -1;
        }
        if (c == '{') {
            while ((c = getc(f)) != '}');
            c = getc(f);
        }
        if (c == '.') {
            while (!pgn_piece_or_rank((c = getc(f))));
        }
        if (c == '?' || c == '!') {
            while ((c = getc(f)) == '?' || c == '!');
            break;
        }
        if (c == ';') {
            while ((c = getc(f)) != '\n');
            c = getc(f);
        }
        if (c == '\n') {
            c = getc(f);
        }
        if (c != ' '){
            buff[i++] = c;
            len++;
        } else {
            while ((c = getc(f)) == ' ');
            ungetc(c, f);
        }
    }
    return len;
}


bool
file_check_nextc(FILE *f, char c)
{
    char t = getc(f);
    if (t != EOF) {
        ungetc(t, f);
    } else {
        printf("EOF!!!!");
    }
    return (t == c);
}

PGN_Error
strip_tag_pairs(FILE *f)
{
    fpos_t indicator;
    char c1 = '\0';

    while ((c1 = getc(f)) != EOF) {
        if (c1 == '[') {
            do {
                c1 = getc(f);
            } while (c1 != ']' && c1 != EOF);
            if (c1 == EOF) {
                PGN_LOG_ERROR(PGN_ERR_TAG_BRACE);
                return PGN_ERR_TAG_BRACE;
            }
            fgetpos(f, &indicator);
        }
    }
    fsetpos(f, &indicator);
    while ((c1 = getc(f)) != '1');
    ungetc(c1, f);
    return PGN_SUCCESS;
}

bool
pgn_is_piece(char c)
{
    return (c == 'N' || c == 'B' || c == 'R' || c == 'Q' || c == 'K' || c == 'O');
}

bool
pgn_is_rank(char c)
{
    return (c == 'a' || c == 'b' || c == 'c' || c == 'd' || 
        c == 'e' || c == 'f' || c == 'g' || c == 'h'
    );
}

bool
pgn_piece_or_rank(char c)
{
    return (pgn_is_piece(c) || pgn_is_rank(c));
}

const char*
pgn_get_error(PGN_Error e)
{
    switch (e) {
        case PGN_ERR_FILE_OPEN: return "couldn't open file";
        case PGN_ERR_TAG_BRACE: return "check if missing ']' in tag pairs";
        case PGN_ERR_STRIP_TAG: return "couldn't strip tags from file";
        case PGN_ERR_SCANLINE:  return "error scanning turn while reading pgn data";
        default: return "...";
    }
}
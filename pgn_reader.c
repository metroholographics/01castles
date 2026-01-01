#include <stdio.h>
#include "pgn_reader.h"

int
main(void)
{
    PGN_Game g;
    if (pgn_create_game(&g, "example_pgn/1examplepgn.txt") < 0) {
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

    

    return PGN_SUCCESS;
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

const char*
pgn_get_error(PGN_Error e)
{
    switch (e) {
        case PGN_ERR_FILE_OPEN: return "couldn't open file";
        case PGN_ERR_TAG_BRACE: return "check if missing ']' in tag pairs";
        case PGN_ERR_STRIP_TAG: return "couldn't strip tags from file";
        default: return "...";
    }
}
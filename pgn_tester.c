#include "castles.h"
#include "string.h"
#include "stdlib.h"

typedef struct {
    char *path;
    int  num_turns;
} Test_Game;

Test_Game tests[] = {
    {"example_pgn/1examplepgn.txt",  36},
    {"example_pgn/2examplepgn.txt",  30},
    {"example_pgn/3examplepgn.txt",  42},
    {"example_pgn/4examplepgn.txt",  61},
    {"example_pgn/5examplepgn.txt",  98},
    {"example_pgn/6examplepgn.txt",  50},
    {"example_pgn/7examplepgn.txt",  199},
    {"example_tests/1.txt",  45},
    {"example_tests/2.txt",  45},
    {"example_tests/3.txt",  46},
    {"example_tests/4.txt",  38},
    {"example_tests/5.txt",  31},
    {"example_tests/6.txt",  60},
    {"example_tests/7.txt",  34},
    {"example_tests/8.txt",  21},
    {"example_tests/9.txt",  37},
    {"example_tests/10.txt", 20},
    {"example_tests/11.txt", 41},
    {"example_tests/12.txt", 43},
    {"example_tests/13.txt", 85},
    {"example_tests/14.txt", 47},
    {"example_tests/15.txt", 48},
    {"example_tests/16.txt", 48},
    {"example_tests/17.txt", 51},
    {"example_tests/18.txt", 39},
    {"example_tests/19.txt", 28},
    {"example_tests/20.txt", 18},
    {"example_tests/21.txt", 44},
    {"example_tests/22.txt", 61}
};


const int num_examples = ARRAY_SIZE(tests);

int main(void)
{
    TurnHistory *games = (TurnHistory*) calloc(num_examples, sizeof(TurnHistory));

    for (int i = 0; i < num_examples; i++) {
        printf("%s\n", tests[i].path);
    }

    for (int i = 0; i < num_examples; i++) {
        PGN_Game g;
        printf("\nloading %s\n", tests[i].path);
        if (pgn_create_game(&g, tests[i].path) < 0) {
            printf("!!Error: could not parse provided PGN\n");
            break;
        } else if (g.num_turns != tests[i].num_turns){
            printf("!!Error: turn mismatch`n");
            break;
        } else {
            printf("...Congrats, PGN parsed\n");
        }

        TurnHistory g_turn = games[i];
        if (store_game_in_boards(&g_turn, g) != CSTL_SUCCESS) {
            break;
        };
    }

    free(games);
    return 0;
}

#include "castles.h"
#include "string.h"
#include "stdlib.h"

const char *example_pgn_paths[] = {
    "example_tests/1.txt",
    "example_tests/2.txt",
    "example_tests/3.txt",
    "example_tests/4.txt",
    "example_tests/5.txt",
    "example_tests/6.txt",
    "example_tests/7.txt",
    "example_tests/8.txt",
    "example_tests/9.txt",
    "example_tests/10.txt",
    "example_tests/11.txt",
    "example_tests/12.txt",
    "example_tests/13.txt",
    "example_tests/14.txt",
    "example_tests/15.txt",
    "example_tests/16.txt",
    "example_tests/17.txt",
    "example_tests/18.txt",
    "example_tests/19.txt",
    "example_tests/20.txt"
};

const int num_examples = ARRAY_SIZE(example_pgn_paths);


int main(void)
{
    printf("%lld\n", sizeof(int));
    TurnHistory *games = (TurnHistory*) calloc(num_examples, sizeof(TurnHistory));

    for (int i = 0; i < num_examples; i++) {
        printf("%s\n", example_pgn_paths[i]);
    }

    for (int i = 0; i < num_examples; i++) {
        PGN_Game g;
        printf("\nloading %s\n", example_pgn_paths[i]);
        if (pgn_create_game(&g, example_pgn_paths[i]) < 0) {
            printf("!!Error: could not parse provided PGN\n");
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

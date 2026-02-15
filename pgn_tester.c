#include "castles.h"
#include "string.h"


const char *example_pgn_paths[] = {
    "example_pgn/1examplepgn.txt",
    "example_pgn/2examplepgn.txt",
    "example_pgn/3examplepgn.txt",
    "example_pgn/4examplepgn.txt",
    "example_pgn/5examplepgn.txt",
    "example_pgn/6examplepgn.txt",
    "example_pgn/7examplepgn.txt",
};

const int num_examples = ARRAY_SIZE(example_pgn_paths);


int main(void)
{
    TurnHistory games[num_examples];
    memset(games, 0, sizeof(TurnHistory));
    for (int i = 0; i < num_examples; i++) {
        printf("%s\n", example_pgn_paths[i]);
    }

    for (int i = 0; i < num_examples; i++) {
        PGN_Game g;
        printf("loading %s\n", example_pgn_paths[i]);
        if (pgn_create_game(&g, example_pgn_paths[i]) < 0) {
            printf("!!Error: could not parse provided PGN\n");
            return -1;
        } else {
            printf("...Congrats, PGN parsed\n");
        }

        TurnHistory g_turn = games[i];
        store_game_in_boards(&g_turn, g);
    }

    return 0;
}

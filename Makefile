CC = gcc
C_FLAGS = -Wall -Wextra -Wpedantic -std=c99 -g
C_FILES = castles.c pgn_reader.c
EXE = main
LINK_FLAGS = -lSDL3 -lSDL3_image


all:
	$(CC) $(C_FLAGS) $(C_FILES) -o $(EXE) $(LINK_FLAGS)

pgn:
	$(CC) $(C_FLAGS) pgn_reader.c -o pgn

test:
	$(CC) $(C_FLAGS) pgn_tester.c $(C_FILES) -o test $(LINK_FLAGS)
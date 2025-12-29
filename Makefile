CC = gcc
C_FLAGS = -Wall -Wextra -Wpedantic -std=c99
C_FILES = *.c
EXE = main
LINK_FLAGS = -lSDL3


all:
	$(CC) $(C_FLAGS) $(C_FILES) -o $(EXE) $(LINK_FLAGS)
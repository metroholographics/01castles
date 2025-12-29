#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


int main(int argc, char *argv[])
{
    (void)argc;(void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initialise SDL\n");
        return -1;
    }

    printf("Hello world\n");

    SDL_Quit();
    return 0;
}
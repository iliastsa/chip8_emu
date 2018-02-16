#include <SDL.h>
#include "../include/chip8.h"

int main(int argc, char **argv){
    if(argc != 2)
        return -1;

    chip8 my_c8;

    my_c8.initialize();

    if(my_c8.loadGame(argv[1]))
        my_c8.run();

    return 0;
}

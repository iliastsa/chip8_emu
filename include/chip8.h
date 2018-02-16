#ifndef CHIP8_H
#define CHIP8_H

#include <SDL.h>

/*
 * Memory map:
 *
 * 0x000-0x1FF => Chip8 interpreter containing font set
 * 0x050-0x0A0 => 4x5 pixel font set
 * 0x200-0xFFF => Program ROM and RAM
 *
 */

class chip8{
    private:
        // 16-bit opcode
        unsigned short opcode;

        // 4KB memory
        unsigned char memory[4096];

        // 15 8-bit GP registers, plus one used as a carry flag
        unsigned char V[16];

        // Index register and program counter
        unsigned short I;
        unsigned short PC;

        // Graphics; 64 x 32 b/w screen
        unsigned char gfx[64 * 32];
        
        // Simple flag for drawing
        bool drawFlag;

        // Timers
        unsigned char delay_timer;
        unsigned char sound_timer;

        // Stack and stack pointer
        unsigned short stack[16];
        unsigned short sp;

        // 16 key keypad
        unsigned char key[16];

        const unsigned char fontset[80] = 
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F        
        };

        // SDL stuff
        SDL_Renderer *renderer;
        SDL_Window *window;
        bool running;

        // Used as CPU core
        void emulateCycle();
        
        // Used for drawing
        void drawScreen();

        // Used for I/O
        void storeKeys();

    public:
        chip8();
        ~chip8();

        void initialize();
        char loadGame(char *filename);
        void run();
};

#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
#include "../include/chip8.h"

using namespace std;

void chip8::initialize(){
    // Clear all registers/memory and set inital PC to 0x200 
    opcode = 0x200;

    for(int i = 0; i < 4096; ++i)
        memory[i] = 0;

    for(int i = 0; i < 16; ++i)
        V[i] = 0;

    I  = 0;
    PC = 0;

    for(int i = 0; i < 64 * 32; ++i)
        gfx[i] = 0;
    
    delay_timer = 0;
    sound_timer = 0;

    for(int i = 0; i < 16; ++i)
        stack[i] = 0;

    sp = 0;

    for(int i = 0; i < 16; ++i)
        key[i] = 0;

    // Finally load font set into memory
    for(int i = 0; i < 80; ++i)
        memory[i] = fontset[i];
}

streampos fileSize(const char *filename){
    streampos fsize = 0;
    ifstream file(filename, ios::binary);

    fsize = file.tellg();
    file.seekg(0, ios::end);
    fsize = file.tellg() - fsize;

    file.close();
    
    return fsize;
}

char chip8::loadGame(char *filename){
    ifstream infile(filename, ios::in | ios::binary);

    if(!infile){
        cerr << "Error reading file " << filename << "." << endl;
        return -1;
    }

    // Check if binary can fit in memory
    if(fileSize(filename) > 4096 - 512){
        cerr << "Error reading file " << filename << ". Binary too large." << endl;
        infile.close();
        return -1;
    }


    // Start reading the binary into memory, offset by 0x200
    int i = 0;
    while(!infile.eof() && i < 4096)
        infile >> memory[i++ + 512];

    infile.close();
    return 1;
}
void chip8::emulateCycle(){
    opcode = memory[PC] << 8 | memory[PC + 1];

    switch(opcode & 0xF000){
        case 0x0000:
            switch(opcode & 0x0FFF){
                // Clear screen
                case 0x00E0:
                    PC++;

                    break;

                case 0x00EE:
                    if(sp == 0)
                        cerr << "Stack underflow at instr " << PC << endl;

                    PC = stack[--sp];

                    break;

                // Call programm at address NNN
                default:
                    break;
            }
            break;

        // Jump to address 0x0NNN
        case 0x1000:
            PC = opcode & 0x0FFF;
            break;

        // Call subroutine at address 0x0NNN (remember to set stack)
        case 0x2000:
            // Stack overflow
            if (sp == 16)
                cerr << "Stack overflow at instr " << PC << endl;

            // Set return address is the next instruction and increment sp
            stack[sp++] = PC + 1;

            // Jump to subroutine
            PC = opcode & 0x0FFF;

            break;

        // Skip next instruction if VX == NN (0x3XNN)
        case 0x3000:
            if(V[opcode & 0x0F00 >> 8] == (opcode & 0x00FF))
                PC++;

            PC++;

            break;

        // Unknown opcode
        default:
            cerr << "Unknown opcode " << opcode << " at " << PC << endl;
    }
}













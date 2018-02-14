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
                    PC +=2;

                    break;

                case 0x00EE:
                    if(sp == 0)
                        cerr << "Stack underflow at instr " << PC << endl;

                    PC = stack[--sp];

                    break;

                // Call programm at address NNN (not needed)
                default:
                    cerr << "Unknown opcode " << opcode << " at " << PC << endl;
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
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                PC +=2;

            PC +=2;

            break;
        
        // Skip next instruction if VX != NN (0x4XNN)
        case 0x4000:
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                PC +=2;

            PC +=2;

            break;

        // Skip next instruction if VX == VY (0x5XY0)
        case 0x5000:
            switch(opcode & 0x000F){
                case 0x0000:
                    if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                        PC +=2;

                    PC +=2;

                    break;

                default:
                    cerr << "Unknown opcode " << opcode << " at " << PC << endl;
            }
            
            break;

        // Const assignment VX = NN (0x6XNN)
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;

            PC +=2;

            break;

        // Const increment VX = NN (0x6XNN)
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;

            PC +=2;

            break;

        // Various ALU ops
        case 0x8000:
            switch((opcode & 0x000F)){
                // Assignment VX = VY (0x8XY0)
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Bitwise OR VX = VX | VY
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Bitwise AND VX = VX & VY
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Bitwise XOR VX = VX ^ VY
                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Addition VX = VX + VY
                case 0x0004:
                    if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Subtraction VX = VX - VY
                case 0x0005:
                    if(V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Shift VY and assign to VX, set VF to the LSB before of VY before the shift
                case 0x0006:
                    V[0xF] = V[(opcode & 0x00F0) >> 4] & 0x01;

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] >>= 1; 

                    PC += 2;

                    break;

                // Subtraction VX = VY - VX
                case 0x0007:
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];

                    PC += 2;

                    break;

                // Shift VY and assign to VX, set VF to the MSB before of VY before the shift
                case 0x0008:
                    V[0xF] = (V[(opcode & 0x00F0) >> 4] & 0x80) >> 7;

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] <<= 1; 

                    PC += 2;

                    break;

                default:
                    cerr << "Unknown opcode " << opcode << " at " << PC << endl;
            }

            break;

        case 0x9000:
            switch(opcode & 0x000F){
                // Skip the next instruction if VX != VY (0x9XY0)
                case 0x0:
                    if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                        PC += 2;

                    PC += 2;
                    
                    break;

                default:
                    cerr << "Unknown opcode " << opcode << " at " << PC << endl;
            }

            break;

        // Set I to address NNN (0xANNN)
        case 0xA000:
            I = opcode & 0x0FFF;
            
            PC += 2;

            break;

        // Set PC to V0 + NNN (0xBNNN)
        case 0xB000:
            PC = V[0] + (opcode & 0x0FFF);

            break;

        // Set VX to rand()%NN (0xCXNN), rand() in [0, 255]
        case 0xC000:
            V[(opcode & 0x0F00) >> 8] = (((unsigned char)rand()) % 256) & (opcode & 0x00FF);

            PC += 2;

            break;

        // Draw
        case 0xD000:
            for(int i = 0; i < (opcode & 0x000F); ++i){
                char pixel_row = memory[I];
                for(int j = 0; j < 8; ++j){

                }
            }

        // Unknown opcode
        default:
            cerr << "Unknown opcode " << opcode << " at " << PC << endl;
    }
}













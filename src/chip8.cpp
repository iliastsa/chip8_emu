#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <SDL.h>
#include "../include/chip8.h"

using namespace std;

chip8::chip8() : running(true){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(640, 320, 0, &window, &renderer);

    SDL_RenderSetLogicalSize(renderer, 64, 32);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

void chip8::initialize(){
    // Clear all registers/memory and set inital PC to 0x200 
    PC = 0x200;

    for(int i = 0; i < 4096; ++i)
        memory[i] = 0;

    for(int i = 0; i < 16; ++i)
        V[i] = 0;

    I  = 0;

    for(int i = 0; i < 64 * 32; ++i)
        gfx[i] = 0;

    drawFlag = false;

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

    srand(time(NULL));
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
    cout << "Loading file: " << filename << endl;
		
	// Open file
	FILE *input = fopen(filename, "rb");

	if (input == NULL){
        cerr << ios::hex << "Error reading file" << endl;
		return 0;
	}

	// Get filesize
    size_t filesize = fileSize(filename);
    cout << "Filesize: " << filesize << endl;
	
	char * buffer = (char*)malloc(sizeof(char) * filesize);
	if (buffer == NULL) 
		return 0;

	// Read file and store it in a buffer
	size_t result = fread (buffer, 1, filesize, input);
	if (result != filesize) {
        cerr << ios::hex << "Error while reading file" << endl;
		return 0;
	}

	// Read each byte from the buffer and store it in memory.
	if(filesize < (4096 - 512))
		for(size_t i = 0; i < filesize; ++i)
			memory[i + 512] = buffer[i];
	else
		cout << "Error: ROM too large to fit in memory" << endl;
	
	// Close file, free buffer
	fclose(input);
	free(buffer);

    return 1;
}
void chip8::emulateCycle(){
    opcode = memory[PC] << 8 | memory[PC + 1];

    switch(opcode & 0xF000){
        case 0x0000:
            switch(opcode & 0x000F){
                // Clear screen
                case 0x0000:
                    for(int i = 0; i < 64 * 32; ++i)
                        gfx[i] = 0;

                    drawFlag = true;

                    PC +=2;

                    break;

                case 0x000E:
                    if(sp == 0)
                        cerr << ios::hex << "Stack underflow at instr " << PC << endl;

                    PC = stack[--sp];

                    break;

                    // Call programm at address NNN (not needed)
                default:
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
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
                cerr << ios::hex << "Stack overflow at instr " << PC << endl;

            // Set return address is the next instruction and increment sp
            stack[sp++] = PC + 2;

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
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
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
                    if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;

                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];

                    PC += 2;

                    break;

                // Shift VY and assign to VX, set VF to the LSB before of VY before the shift
                case 0x0006:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x01;

                    V[(opcode & 0x0F00) >> 8] >>= 1; 

                    PC += 2;

                    break;

                // Subtraction VX = VY - VX
                case 0x0007:
                    if(V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0;
                    else
                        V[0xF] = 1;

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];

                    PC += 2;

                    break;

                // Shift VY and assign to VX, set VF to the MSB before of VY before the shift
                case 0x000E:
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;

                    V[(opcode & 0x0F00) >> 8] <<= 1; 

                    PC += 2;

                    break;

                default:
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
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
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
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
            V[(opcode & 0x0F00) >> 8] = (((unsigned char)rand()) % 0xFF) & (opcode & 0x00FF);

            PC += 2;

            break;

        // Draw sprite 8xN at memory address I at postion (VX, VY) (0xDXYN)
        case 0xD000:
            {
                // Coordinates
                unsigned char x = V[(opcode & 0x0F00) >> 8];
                unsigned char y = V[(opcode & 0x00F0) >> 4];

                V[0xF] = 0;

                // Loop through the pixels
                for(int i = 0; i < (opcode & 0x000F); ++i){
                    unsigned char pixel_row = memory[I + i];
                    unsigned char mask = 0x80;

                    for(int j = 0; j < 8; ++j){
                        if((pixel_row & mask) != 0){
                            if(gfx[(x + j) + (y + i) * 64] == 1)
                                V[0xF] = 1;

                            gfx[(x + j) + (y + i) * 64] ^= 1;
                        }
                        mask >>= 1;
                    }
                }

                drawFlag = true;

                PC += 2;
            }

            break;

        case 0xE000:
            switch(opcode & 0x00FF){
                // Skip next instruction if key stored in VX is pressed (0xEX9E)
                case 0x009E:
                    if(key[V[(opcode & 0x0F00) >> 8]] != 0)
                        PC += 2;

                    PC += 2;

                    break;

                // Skip next instruction if key stored in VX is not pressed (0xEXA1)
                case 0x00A1:
                    if(key[V[(opcode & 0x0F00) >> 8]] == 0)
                        PC += 2;

                    PC += 2;

                    break;

                default:
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
            }

            break;

        case 0xF000:
            switch(opcode & 0x00FF){
                // Set VX register to the delay timer value
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = delay_timer;

                    PC += 2;

                    break;

                // Wait for key press, and store it in VX
                case 0x000A:
                    {
                        bool key_pressed = false;
                        for(int i = 0; i < 16; ++i){
                            if(key[i] != 0){
                                V[(opcode & 0x0F00) >> 8] = i;
                                key_pressed = true;
                            }
                        }

                        if(!key_pressed)
                            return;

                        PC += 2;
                    }
                    break;

                // Set delay timer to VX
                case 0x0015:
                    delay_timer = V[(opcode & 0x0F00) >> 8];

                    PC += 2;

                    break;

                // Set delay timer to VX
                case 0x0018:
                    sound_timer = V[(opcode & 0x0F00) >> 8];

                    PC += 2;

                    break;

                // Add VX to I
                case 0x001E:
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;

                    I += V[(opcode & 0x0F00) >> 8];

                    PC += 2;

                    break;

                // Store location of VX character in I
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8] * 5;

                    PC += 2;

                    break;

                // Store BCD representation in I, I+1 and I+2
                case 0x0033:
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] % 100) / 10;
                    memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;

                    PC += 2;

                    break;

                // Store V0-VX in memory starting at I, incrementing I for each value stored
                case 0x0055:
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        memory[I++] = V[i];

                    PC += 2;

                    break;

                // Load V0-VX from memory starting at I, incrementing I for each value stored
                case 0x0065:
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        V[i] = memory[I++];

                    PC += 2;

                    break;

                default:
                    cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
            }

            break;

            // Unknown opcode
        default:
            cerr << ios::hex << "Unknown opcode " << opcode << " at " << PC << endl;
    }

    if(delay_timer > 0)
        --delay_timer;

    if(sound_timer > 0){
        if(sound_timer == 1){
            //BEEP
        }
        --sound_timer;
    }
}

void chip8::drawScreen(){
    for(int y = 0; y < 32; ++y){
        for(int x = 0; x < 64; ++x){
            if(gfx[y * 64 + x] == 1)
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            else
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_RenderPresent(renderer);
}

void chip8::storeKeys(){
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        int value;
        switch(event.type){
            case SDL_KEYDOWN: value = 1; break;
            case SDL_KEYUP  : value = 0; break;
            case SDL_QUIT   : running = false;
            default         : continue;
        }

        switch(event.key.keysym.sym){
            case SDLK_x: key[0 ] = value; break;
            case SDLK_1: key[1 ] = value; break;
            case SDLK_2: key[2 ] = value; break;
            case SDLK_3: key[3 ] = value; break;
            case SDLK_q: key[4 ] = value; break;
            case SDLK_w: key[5 ] = value; break;
            case SDLK_e: key[6 ] = value; break;
            case SDLK_a: key[7 ] = value; break;
            case SDLK_s: key[8 ] = value; break;
            case SDLK_d: key[9 ] = value; break;
            case SDLK_z: key[10] = value; break;
            case SDLK_c: key[11] = value; break;
            case SDLK_4: key[12] = value; break;
            case SDLK_r: key[13] = value; break;
            case SDLK_f: key[14] = value; break;
            case SDLK_v: key[15] = value; break;
        }
    }
}

void chip8::run(){
    while(running){
        usleep(1000);
        drawFlag = false;

        emulateCycle();

        if(drawFlag)
            drawScreen();

        storeKeys();
    }
}

chip8::~chip8(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}












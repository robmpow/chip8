//
// Created by rmpowell on 10/3/18.
//

#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "chip8.h"
#include "chip8_util.h"

/*
 *      Memory Map
 *  _____ 0x0000 _____
 * |        *         |
 * |        *         |
 * |   Main Memory    |
 * |        *         |
 * |        *         |
 * |_____ 0x1000 _____|
 * |   V Reg (0-F)    |
 * |_____ 0x1010 _____|
 * |      Reg I       |
 * |_____ 0x1012 _____|
 * |    Reg DT/ST     |
 * |_____ 0x1014 _____|
 * |      Stack       |
 * |_____ 0x1034 _____|
 * |  Display Bitmap  |
 * |_____ 0x1834 _____|
 */

extern uint8_t* allocate();
extern void deallocate(uint8_t* mem);

chip8::chip8(uint seed, chip8io* io){
    this->seed = seed;
    this->memory = allocate();
    this->stack = (uint16_t*) (memory + STACK_OFFSET);
    this->v_regs = memory + V_REG_OFFSET;
    this->i_reg = (uint16_t*) (memory + I_REG_OFFSET);
    this->dt_reg = memory + DT_REG_OFFSET;
    this->st_reg = memory + ST_REG_OFFSET;
    this->disp = memory + DISP_OFFSET;
}   

chip8::~chip8(){
    deallocate(this->memory);
}

void chip8::reset() {
    //memcpy(memory, sprite_table, 0x50);
    memset(memory, 's', SPRITE_TABLE_SIZE);
    memset(memory + PROG_START_OFFSET, 'm', MAIN_MEM_SIZE - PROG_START_OFFSET);
    memset((uint8_t*) stack, 's', STACK_SIZE);
    memset(v_regs, 'v' , NUM_V_REG);
    *i_reg = ('i' << 8) | 'i';
    *dt_reg = 't';
    *st_reg = 't';
    memset(memory + DISP_OFFSET, 'd', DISP_SIZE);
    program_counter = 0x200;
    stack_pointer = -1;
    srand(seed);
}

void chip8::load(std::string file_path){
    std::ifstream file_stream;
    file_stream.open(file_path);
    file_stream.read((char*) memory, MAIN_MEM_SIZE);
}

/*
 * Vx  : register addressed by the lower 4 bits of upper byte of op
 * Vy  : register addressed by the upper 4 bits of lower byte of op
 * nnn : 12 least significant bits of op (3 least significant nibbles)
 * kk  : lower byte of op
 * nib : least significant 4 bits of op (nibble)
 */

void chip8::run_tick() {

    uint16_t op = *((uint16_t*)  (memory + PROG_START_OFFSET + program_counter));
    switch(op & 0xFF00){
        case 0x0000:
            switch(op & 0x0FFF){
                case 0x0E0:
                    //clear display
                    memset(disp, 0, DISP_Y * (DISP_X / sizeof(uint8_t)));
                    break;
                case 0x0EE:
                    //ret
                    if(stack_pointer == -1){
                        string error_message = "Ret with empty stack at address <0x" + to_string(program_counter + PROG_START_OFFSET) + ">.\n";
                        throw error_message;
                    }
                    program_counter = stack[stack_pointer--];
                    break;
                default:
                    string error_message = "Unknown opcode <0x" + to_string(op) + "> at address <0x" + to_string(PROG_START_OFFSET + program_counter) + ">.\n";
                    throw error_message;

            }
            break;
        case 0x1000:
            program_counter = nnn;
            break;
        case 0x2000:
            stack[++stack_pointer] = program_counter;
            program_counter = nnn;
            break;
        case 0x3000:
            if(Vx == kk){
                program_counter+=2;
            }
            break;
        case 0x4000:
            if(Vx != kk){
                program_counter+=2;
            }
            break;
        case 0x5000:
            if(Vx == v_regs[Vy]){
                program_counter+=2;
            }
            break;
        case 0x6000:
            Vx = kk;
            break;
        case 0x7000:
            Vx += kk;
            break;
        case 0x8000:
            switch(nib){
                case 0:
                    Vx = Vy;
                    break;
                case 1:
                    Vx |= Vy;
                    break;
                case 2:
                    Vx &= Vy;
                    break;
                case 3:
                    Vx ^= Vy;
                    break;
                case 4:
                    Vx |= Vy;
                    break;
                case 5:
                    Vx |= Vy;
                    break;
                case 6:
                    v_regs[0xF] = static_cast<uint8_t>(Vx & 0x1);
                    Vx >>= 2;
                    break;
                case 7:
                    v_regs[0xF] = static_cast<uint8_t>(Vy > Vx);
                    Vx = Vy - Vx;
                    break;
                case 0xE:
                    v_regs[0xF] = static_cast<uint8_t>((Vx & 0x80)? 1 : 0);
                    Vx >>= 2;
                default:
                    string error_message = "Unknown opcode <0x" + to_string(op) + "> at address <0x" + to_string(PROG_START_OFFSET + program_counter) + ">.\n";
                    throw error_message;            }
            break;
        case 0x9000:
            if(Vx != Vy){
                program_counter+=2;
            }
            break;
        case 0xA000:
            *i_reg = nnn;
            break;
        case 0xB000:
            program_counter = nnn + v_regs[0];
            break;
        case 0xC000:
            Vx = rand() & kk;
            break;
        case 0xD000:
            //draw screen
            break;
        case 0xE000:
            switch(kk){
                case 0x9E:
                    //check if key pressed
                    break;
                case 0xA1:
                    //check if key not pressed
                    break;
                default:
                    string error_message = "Unknown opcode <0x" + to_string(op) + "> at address <0x" + to_string(PROG_START_OFFSET + program_counter) + ">.\n";
                    throw error_message;            
            }
            break;
        case 0xF000:
            switch(kk){
                case 0x07:
                    Vx = *dt_reg; 
                    break;
                case 0x0A:
                    //wait for key press
                    break;
                case 0x15:
                    *dt_reg = Vx;
                    break;
                case 0x18:
                    *st_reg = Vx;
                    break;
                case 0x1E:
                    *i_reg += Vx;
                    break;
                case 0x29:
                    *i_reg = Vx * 5;
                    break;
                case 0x33:
                    memory[*i_reg] = Vx / 100;
                    memory[*i_reg +1] = (Vx % 100) / 10;
                    memory[*i_reg + 2] = Vx % 10;
                    break;
                case 0x55:
                    for(uint8_t i = 0; i < 0x10; i++)
                        memory[*i_reg + i] = V(i); 
                    break;
                case 0x65:
                    for(uint8_t i = 0; i < 0x10; i++)
                        V(i) = memory[*i_reg + i]; 
                    break;
                default:
                    string error_message = "Unknown opcode <0x" + to_string(op) + "> at address <0x" + to_string(PROG_START_OFFSET + program_counter) + ">.\n";
                    throw error_message;
            }
            break;
        default:
                    string error_message = "Unknown opcode <0x" + to_string(op) + "> at address <0x" + to_string(PROG_START_OFFSET + program_counter) + ">.\n";
                    throw error_message;
        }
    program_counter+=2;
}

void chip8::write(uint16_t address, uint8_t value){
    //TODO
}

uint8_t chip8::read(uint16_t address){
    //TODO
    return 0;
}

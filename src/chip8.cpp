//
// Created by rmpowell on 10/3/18.
//

#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "chip8.h"
#include "util.h"

chip8::chip8(uint seed, chip8io io){
    this->seed = seed;
}

void chip8::reset() {
    memcpy(memory, sprite_table, 0x50);
    memset(memory + PROG_START, 0, MEMORY_SIZE - PROG_START);
    memset(stack, 0, STACK_SIZE);
    memset(registers, 0 , NUM_REGISTERS);
    register_i = 0;
    register_dt = 0;
    register_st = 0;
    program_counter = 0x200;
    stack_pointer = -1;
    register_i = 0;
    register_dt = 0;
    register_st = 0;
    srand(seed);
}

void chip8::load(std::string file_path){
    std::ifstream file_stream;
    file_stream.open(file_path);
    file_stream.read((char*) memory, MEMORY_SIZE);
}

/*
 * Vx  : register addressed by the lower 4 bits of upper byte of op
 * Vy  : register addressed by the upper 4 bits of lower byte of op
 * nnn : 12 least significant bits of op (3 least significant nibbles)
 * kk  : lower byte of op
 * nib : least significant 4 bits of op (nibble)
 */

void chip8::run_tick() {
    uint16_t op = *((uint16_t*)  (memory + PROG_START + program_counter));
    switch(op & 0xFF00){
        case 0x0000:
            switch(op & 0x0FFF){
                case 0x0E0:
                    //clear display
                    memset(display, 0, DISPLAY_Y * (DISPLAY_X / sizeof(uint8_t)));
                    break;
                case 0x0EE:
                    //ret
                    if(stack_pointer == -1){
                        throw build_err_message("Ret with empty stack at address <0x%hx>.\n");
                    }
                    program_counter = stack[stack_pointer--];
                    break;
                default:
                    throw build_err_message("Unknown opcode <0x%hx> at address <0x%hx>.\n", op, PROG_START + program_counter);

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
            if(Vx == registers[Vy]){
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
                    registers[0xF] = static_cast<uint8_t>(Vx & 0x1);
                    Vx >>= 2;
                    break;
                case 7:
                    registers[0xF] = static_cast<uint8_t>(Vy > Vx);
                    Vx = Vy - Vx;
                    break;
                case 0xE:
                    registers[0xF] = static_cast<uint8_t>((Vx & 0x80)? 1 : 0);
                    Vx >>= 2;
                default:
                    throw build_err_message("Unknown opcode <0x%hx> at address <0x%hx>.\n", op, PROG_START + program_counter);
            }
            break;
        case 0x9000:
            if(Vx != Vy){
                program_counter+=2;
            }
            break;
        case 0xA000:
            register_i = nnn;
            break;
        case 0xB000:
            program_counter = nnn + registers[0];
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
                    throw build_err_message("Unknown opcode <0x%hx> at address <0x%hx>.\n", op, PROG_START + program_counter);
            }
            break;
        case 0xF000:
            switch(kk){
                case 0x07:
                    Vx = register_dt; 
                    break;
                case 0x0A:
                    //wait for key press
                    break;
                case 0x15:
                    register_dt = Vx;
                    break;
                case 0x18:
                    register_st = Vx;
                    break;
                case 0x1E:
                    register_i += Vx;
                    break;
                case 0x29:
                    register_i = Vx * 5;
                    break;
                case 0x33:
                    memory[register_i] = Vx / 100;
                    memory[register_i + 1] = (Vx % 100) / 10;
                    memory[register_i + 2] = Vx % 10;
                    break;
                case 0x55:
                    for(uint8_t i = 0; i < 0x10; i++){
                        memory[register_i + i] = V(i); 
                    }
                    break;
                case 0x65:
                    for(uint8_t i = 0; i < 0x10; i++){
                        V(i) = memory[register_i + i]; 
                    }
                    break;
                default:
                    throw build_err_message("Unknown opcode <0x%hx> at address <0x%hx>.\n", op, PROG_START + program_counter);
            }
            break;
        default:
            throw build_err_message("Unknown opcode <0x%hx> at address <0x%hx>.\n", op, PROG_START + program_counter);
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
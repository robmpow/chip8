//
// Created by rmpowell on 10/3/18.
//

#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <endian.h>
#include <iomanip>

#include "bitfield.h"
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



chip8::chip8(uint seed, std::string file_name) : dist(0, 255){
    this->seed = seed;
    reset(seed);
    load(file_name);
}

chip8::chip8(uint seed, std::istream& file_stream) : dist(0, 255){
    this->seed = seed;
    reset(seed);
    load(file_stream);
}

void chip8::reset(){
    reset(this->seed);
}

void chip8::reset(uint seed) {
    // D(debugMsg("RESET\n"));
    memcpy(memory, sprite_table, 0x50);
    memset(memory + PROG_START_OFFSET, 0, MAIN_MEM_SIZE - PROG_START_OFFSET);
    memset((uint8_t*) stack, 0, STACK_SIZE * 2);
    memset(v_regs, 0 , NUM_V_REG);
    i_reg = 0;
    dt_reg = 0;
    st_reg = 0;
    t_counter = 0;
    memset(disp, 0, DISP_SIZE);
    program_counter = PROG_START_OFFSET;
    stack_pointer = -1;
    generator = std::mt19937(seed);
}

void chip8::load(std::string file_path){
    std::ifstream file_stream;
    file_stream.open(file_path);
    load(file_stream);
}

void chip8::load(std::istream& in_stream){
    in_stream.read((char*) (memory + PROG_START_OFFSET), MAIN_MEM_SIZE - PROG_START_OFFSET);
}

void chip8::update_keystate(bool press_state, bool repeat, chip8_key key){
    if(key != KEY_NULL)
        key_states = (key_states & ~(0x1 << key)) | (press_state << key & (0x1 << key));
}

/*
 * VX  : register addressed by the lower 4 bits of upper byte of op
 * VY  : register addressed by the upper 4 bits of lower byte of op
 * NNN : 12 least significant bits of op (3 least significant nibbles)
 * KK  : lower byte of op
 * NIB : least significant 4 bits of op (nibble)
 */

tick_result chip8::run_tick() {

    tick_result tick_res;
    tick_res.display_update = 0;
    tick_res.sound_state = 0;

    t_counter++;
    if(t_counter == 10){
        t_counter = 0;
        if(dt_reg){
            dt_reg--;
        }
        if(st_reg){
            st_reg--;
            tick_res.sound_state = 1;
        }
        else{
            tick_res.sound_state = 0;
        }
    }

    uint16_t arith_res;
    #ifdef LITTLE_ENDIAN
        uint16_t op = (memory[program_counter] << 8) | memory[program_counter + 1];
    #else
        uint16_t op = *((uint16_t*)  (memory + program_counter));
    #endif
    switch(op & 0xF000){
        case 0x0000:
            switch(op & 0x0FFF){
                case 0x0E0:
                    //clear display
                    memset(disp, 0, DISP_Y * (DISP_X / 8));
                    program_counter+=2;
                    tick_res.display_update = 1;
                    break;
                case 0x0EE:
                    //ret
                    if(stack_pointer == -1){
                        std::stringstream error_stream;
                        error_stream << "Ret with empty stack at address <0x" << std::hex << std::setfill('0') << std::setw(4) << program_counter<< ">.";
                        throw error_stream.str();
                    }
                    program_counter = stack[stack_pointer--];
                    program_counter += 2;
                    break;
                default:
                    std::stringstream error_stream;
                    error_stream << "Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << program_counter << ">.";
                    throw error_stream.str();

            }
            break;
        case 0x1000:
            program_counter = NNN;
            break;
        case 0x2000:
            stack[++stack_pointer] = program_counter;
            program_counter = NNN;
            break;
        case 0x3000:
            if(VX == KK){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x4000:
            if(VX != KK){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x5000:
            if(VX == VY){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x6000:
            VX = KK;
            program_counter+=2;
            break;
        case 0x7000:
            VX += KK;
            program_counter+=2;
            break;
        case 0x8000:
            switch(NIB){
                case 0:
                    VX = VY;
                    program_counter+=2;
                    break;
                case 1:
                    VX |= VY;
                    program_counter+=2;
                    break;
                case 2:
                    VX &= VY;
                    program_counter+=2;
                    break;
                case 3:
                    VX ^= VY;
                    program_counter+=2;
                    break;
                case 4:
                    arith_res = VX + VY;
                    V(0xF) = (arith_res & 0x100) ? 1 : 0;
                    VX = arith_res & 0xFF;
                    program_counter+=2;
                    break;
                case 5:
                    V(0xF) = (VX > VY)? 1 : 0;
                    VX -= VY;
                    program_counter+=2;
                    break;
                case 6:
                    V(0xF) = VX & 0x1;
                    VX >>= 1;
                    program_counter+=2;
                    break;
                case 7:
                    v_regs[0xF] = (VY > VX)? 1 : 0;
                    VX = VY - VX;
                    program_counter+=2;
                    break;
                case 0xE:
                    v_regs[0xF] = (VX & 0x80)? 1 : 0;
                    VX <<= 1;
                    program_counter+=2;
                    break;
                default:
                    std::stringstream error_stream;
                    error_stream << "Unknown opcode <0x" << std::hex << op << "> at address <0x" << std::hex << program_counter << ">.";
                    throw error_stream.str();           }
            break;
        case 0x9000:
            if(VX != VY){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0xA000:
            i_reg = NNN;
            program_counter+=2;
            break;
        case 0xB000:
            program_counter = NNN + v_regs[0];
            program_counter+=2;
            break;
        case 0xC000:
            VX = dist(generator) & KK;
            program_counter+=2;
            break;
        case 0xD000:
            //draw screen
            for(int i = 0; i < NIB; i++){
                uint8_t sprite_byte = memory[i_reg + i];
                V(0xF) |= (disp[(VX >> 3) + (VY + i) * (DISP_X >> 3)] & (sprite_byte >> (VX & 0x7)))? 1 : 0;
                disp[(VX >> 3) + (VY + i) * (DISP_X >> 3)] ^= (sprite_byte >> (VX & 0x7));
                if(VX & 0x7){
                        V(0xF) |= (disp[(VX >> 3) + (VY + i) * (DISP_X >> 3) + 1] & (sprite_byte << (8 - (VX & 0x7))))? 1 : 0;
                        disp[(VX >> 3) + (VY + i) * (DISP_X >> 3) + 1] ^= (sprite_byte << (8 - (VX & 0x7)));
                }
            }
            tick_res.display_update = 1;
            program_counter+=2;
            break;
        case 0xE000:
            switch(KK){
                case 0x9E:
                    //check if key pressed
                    if((key_states << VX) & 0x1){
                        program_counter+=2;
                    }
                    program_counter+=2;
                    break;
                case 0xA1:
                    //check if key not pressed
                    if(!((key_states << VX) & 0x1)){
                        program_counter+=2;
                    }
                    program_counter+=2;
                    break;
                default:
                    std::stringstream error_stream;
                    error_stream << "Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << program_counter << ">.";
                    throw error_stream.str();            
            }
            break;
        case 0xF000:
            switch(KK){
                case 0x07:
                    VX = dt_reg; 
                    program_counter+=2;
                    break;
                case 0x0A:
                    //wait for key press
                    program_counter+=2;
                    break;
                case 0x15:
                    dt_reg = VX;
                    program_counter+=2;
                    break;
                case 0x18:
                    st_reg = VX;
                    if(st_reg){
                        tick_res.sound_state = 1;
                    }   
                    program_counter+=2;
                    break;
                case 0x1E:
                    i_reg += VX;
                    program_counter+=2;
                    break;
                case 0x29:
                    i_reg = VX * 5;
                    program_counter+=2;
                    break;
                case 0x33:
                    memory[i_reg] = VX / 100;
                    memory[i_reg +1] = (VX % 100) / 10;
                    memory[i_reg + 2] = VX % 10;
                    program_counter+=2;
                    break;
                case 0x55:
                    for(uint8_t i = 0; i < 0x10; i++)
                        memory[i_reg + i] = V(i);
                    program_counter+=2; 
                    break;
                case 0x65:
                    for(uint8_t i = 0; i < 0x10; i++)
                        V(i) = memory[i_reg + i]; 
                    program_counter+=2;
                    break;
                default:
                    std::stringstream error_stream;
                    error_stream << "Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << program_counter << ">.";
                    throw error_stream.str();
            }
            break;
        default:
                    std::stringstream error_stream;
                    error_stream << "Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << program_counter << ">.";
                    throw error_stream.str();
    }

    return tick_res;
}
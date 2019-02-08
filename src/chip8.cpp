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
#include "logger_impl.hpp"

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
    stack_pointer = 0xf;
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

    LOG_TRACE("chip8: key state: 0x", std::hex, std::setw(4), std::setfill('0'), key_states, logger::endl);

    LOG_TRACE("chip8: vregs{0x0,0xf}: ", std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x0)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x1)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x2)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x3)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x4)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x5)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x6)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x7)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x8)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0x9)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xa)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xb)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xc)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xd)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xe)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(V(0xf)), logger::endl);
    
    LOG_TRACE("chip8: DT, ST, I: ", std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(dt_reg), " ",
                                    std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(st_reg), " ",
                                    std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(i_reg), logger::endl);

    LOG_TRACE("chip8: stack pointer: ", std::hex, static_cast<uint>(stack_pointer), logger::endl);

    LOG_TRACE("chip8: stack{0x0, 0xf}: ", std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x0]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x1]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x2]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x3]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x4]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x5]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x6]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x7]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x8]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0x9]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xa]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xb]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xc]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xd]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xe]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(stack[0xf]), logger::endl);

    tick_result tick_res;
    tick_res.display_update = 0;
    tick_res.sound_state = 0;
    tick_res.error = 0;

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
    switch(op & 0xf000){
        case 0x0000:
            switch(op & 0x0fff){
                case 0x0e0:
                    // CLS
                    memset(disp, 0, DISP_Y * (DISP_X / 8));
                    program_counter+=2;
                    tick_res.display_update = 1;
                    break;
                case 0x0ee:
                    //RET
                    if(stack_pointer == 0xf){
                        tick_res.error = 1;
                        LOG_ERROR("chip8: Stack wrap, call stack empty, triggered by RET at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
                    }
                    program_counter = stack[stack_pointer--];
                    program_counter += 2;
                    break;
                default:
                    tick_res.error = 1;
                    LOG_ERROR("chip8: Unknown opcode <0x", std::hex, std::setfill('0'), std::setw(4), op, "> at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
            }
            break;
        case 0x1000:
            // JMP nnn
            program_counter = NNN;
            break;
        case 0x2000:
            // CALL nnn
            if(stack_pointer == 0x0){
                tick_res.error = 1;
                LOG_ERROR("chip8: Stack wrap, call stack full, triggered by CALL at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
            }
            stack[stack_pointer--] = program_counter;
            program_counter = NNN;
            break;
        case 0x3000:
            // SE VX, KK
            if(VX == KK){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x4000:
            //SNE VX, KK
            if(VX != KK){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x5000:
            // SE VX, VY
            if(VX == VY){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0x6000:
            // LD VX, KK
            VX = KK;
            program_counter+=2;
            break;
        case 0x7000:
            // ADD VX, KK
            VX += KK;
            program_counter+=2;
            break;
        case 0x8000:
            switch(NIB){
                case 0:
                    // LD VX, VY
                    VX = VY;
                    program_counter+=2;
                    break;
                case 1:
                    // OR VX, VY
                    VX |= VY;
                    program_counter+=2;
                    break;
                case 2:
                    // AND VX, VY
                    VX &= VY;
                    program_counter+=2;
                    break;
                case 3:
                    // XOR VX, VY
                    VX ^= VY;
                    program_counter+=2;
                    break;
                case 4:
                    // ADD VX,
                    arith_res = VX + VY;
                    V(0xf) = (arith_res & 0x100) ? 1 : 0;
                    VX = arith_res & 0xff;
                    program_counter+=2;
                    break;
                case 5:
                    // SUB VX, VY
                    V(0xf) = (VX > VY)? 1 : 0;
                    VX -= VY;
                    program_counter+=2;
                    break;
                case 6:
                    //SHR VX, Vy
                    V(0xf) = VX & 0x1;
                    VX >>= 1;
                    program_counter+=2;
                    break;
                case 7:
                    //SUBN VX, VY
                    v_regs[0xf] = (VY > VX)? 1 : 0;
                    VX = VY - VX;
                    program_counter+=2;
                    break;
                case 0xe:
                    //SHL VX, VY
                    v_regs[0xf] = (VX & 0x80)? 1 : 0;
                    VX <<= 1;
                    program_counter+=2;
                    break;
                default:
                    tick_res.error = 1;
                    LOG_ERROR("chip8: Unknown opcode <0x", std::hex, std::setfill('0'), std::setw(4), op, "> at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
        }
            break;
        case 0x9000:
            // SNE VX, VY
            if(VX != VY){
                program_counter+=2;
            }
            program_counter+=2;
            break;
        case 0xa000:
            // LD I, NNN
            i_reg = NNN;
            program_counter+=2;
            break;
        case 0xb000:
            // JMP V0, NNN
            program_counter = NNN + v_regs[0];
            program_counter+=2;
            break;
        case 0xc000:
            // RND VX, KK
            VX = dist(generator) & KK;
            program_counter+=2;
            break;
        case 0xd000:
            // DRW VX, VY, NIB
            for(int i = 0; i < NIB; i++){
                uint8_t sprite_byte = memory[i_reg + i];
                V(0xf) |= (disp[(VX >> 3) + (VY + i) * (DISP_X >> 3)] & (sprite_byte >> (VX & 0x7)))? 1 : 0;
                disp[(VX >> 3) + (VY + i) * (DISP_X >> 3)] ^= (sprite_byte >> (VX & 0x7));
                if(VX & 0x7){
                        V(0xf) |= (disp[(VX >> 3) + (VY + i) * (DISP_X >> 3) + 1] & (sprite_byte << (8 - (VX & 0x7))))? 1 : 0;
                        disp[(VX >> 3) + (VY + i) * (DISP_X >> 3) + 1] ^= (sprite_byte << (8 - (VX & 0x7)));
                }
            }
            tick_res.display_update = 1;
            program_counter+=2;
            break;
        case 0xe000:
            switch(KK){
                case 0x9e:
                    // SKP VX
                    if((key_states >> VX) & 0x1){
                        program_counter+=2;
                    }
                    program_counter+=2;
                    break;
                case 0xa1:
                    //SKNP VX
                    if(!((key_states >> VX) & 0x1)){
                        program_counter+=2;
                    }
                    program_counter+=2;
                    break;
                default:
                    tick_res.error = 1;
                    LOG_ERROR("chip8: Unknown opcode <0x", std::hex, std::setfill('0'), std::setw(4), op, "> at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);          
            }
            break;
        case 0xf000:
            switch(KK){
                case 0x07:
                    // LD VX, DT
                    VX = dt_reg; 
                    program_counter+=2;
                    break;
                case 0x0a:
                    // LD VX, KP
                    if(key_states){
                        uint8_t i = 0;
                        while((key_states >> i++) & 0x1){
                            VX = i - 1;
                        }
                        program_counter+=2;
                    } 
                    break;
                case 0x15:
                    // LD DT, VX
                    dt_reg = VX;
                    program_counter+=2;
                    break;
                case 0x18:
                    // LD ST, VX
                    st_reg = VX;
                    if(st_reg){
                        tick_res.sound_state = 1;
                    }   
                    program_counter+=2;
                    break;
                case 0x1e:
                    // ADD I, VX
                    i_reg += VX;
                    program_counter+=2;
                    break;
                case 0x29:
                    // LD I, Sprt(VX)
                    i_reg = VX * 5;
                    program_counter+=2;
                    break;
                case 0x33:
                    // LD I, BCD(VX)
                    memory[i_reg] = VX / 100;
                    memory[i_reg +1] = (VX % 100) / 10;
                    memory[i_reg + 2] = VX % 10;
                    program_counter+=2;
                    break;
                case 0x55:
                    // LD [I], VX
                    for(uint8_t i = 0; i < 0x10; i++)
                        memory[i_reg + i] = V(i);
                    program_counter+=2; 
                    break;
                case 0x65:
                    // LD VX, [I]
                    for(uint8_t i = 0; i < 0x10; i++)
                        V(i) = memory[i_reg + i]; 
                    program_counter+=2;
                    break;
                default:
                    tick_res.error = 1;
                    LOG_ERROR("chip8: Unknown opcode <0x", std::hex, std::setfill('0'), std::setw(4), op, "> at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
            }
            break;
        default:
                tick_res.error = 1;
                LOG_ERROR("chip8: Unknown opcode <0x", std::hex, std::setfill('0'), std::setw(4), op, "> at address <0x", std::hex, std::setfill('0'), std::setw(4), program_counter, ">.", logger::endl);
    }

    return tick_res;
}
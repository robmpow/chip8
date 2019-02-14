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

#include "Bitfield.hpp"
#include "Chip8.hpp"
#include "LoggerImpl.hpp"

namespace Chip8{

Chip8::Chip8(uint seed, const std::string& filePath) : m_dist(0, 255), m_seed(seed) {
    reset(m_seed);
    load(filePath);
}

Chip8::Chip8(uint seed, std::istream& inStream) : m_dist(0, 255), m_seed(seed){
    reset(m_seed);
    load(inStream);
}

void Chip8::reset(){
    reset(m_seed);
}

void Chip8::reset(uint seed) {
    memcpy(m_memory, m_sprite_table, 0x50);
    memset(m_memory + CHIP8_PROG_START_OFFSET, 0, CHIP8_MAIN_MEM_SIZE - CHIP8_PROG_START_OFFSET);
    memset((uint8_t*) m_stack, 0, CHIP8_STACK_SIZE * 2);
    memset(m_vRegs, 0 , CHIP8_NUM_V_REG);
    m_iReg = 0;
    m_dtReg = 0;
    m_stReg = 0;
    m_tCounter = 0;
    memset(m_disp, 0, CHIP8_DISP_SIZE);
    m_programCounter = CHIP8_PROG_START_OFFSET;
    m_stackPointer = 0xf;
    m_generator = std::mt19937(seed);
}

void Chip8::load(const std::string& file_path){
    std::ifstream file_stream;
    file_stream.open(file_path);
    load(file_stream);
}

void Chip8::load(std::istream& inStream){
    inStream.read((char*) (m_memory + CHIP8_PROG_START_OFFSET), CHIP8_MAIN_MEM_SIZE - CHIP8_PROG_START_OFFSET);
}

void Chip8::updateKeystate(bool press_state, bool repeat, const Chip8Key& key){
    if(key != KEY_NULL)
        m_keystates = (m_keystates & ~(0x1 << key)) | (press_state << key & (0x1 << key));
}

/*
 * VX  : register addressed by the lower 4 bits of upper byte of op
 * VY  : register addressed by the upper 4 bits of lower byte of op
 * NNN : 12 least significant bits of op (3 least significant nibbles)
 * KK  : lower byte of op
 * NIB : least significant 4 bits of op (nibble)
 */

TickResult Chip8::run_tick() {


    chip8Logger.log<Logger::LogTrace>("Chip8: key state: 0x", std::hex, std::setw(4), std::setfill('0'), m_keystates, Logger::endl);

    chip8Logger.log<Logger::LogTrace>("Chip8: vregs{0x0,0xf}: ", std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x0)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x1)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x2)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x3)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x4)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x5)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x6)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x7)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x8)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0x9)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xa)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xb)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xc)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xd)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xe)), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(CHIP8_V(0xf)), Logger::endl);
    
    chip8Logger.log<Logger::LogTrace>("Chip8: DT, ST, I: ", std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_dtReg), " ",
                                    std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_stReg), " ",
                                    std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_iReg), Logger::endl);

    chip8Logger.log<Logger::LogTrace>("Chip8: stack pointer: ", std::hex, static_cast<uint>(m_stackPointer), Logger::endl);

    chip8Logger.log<Logger::LogTrace>("Chip8: stack{0x0, 0xf}: ", std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x0]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x1]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x2]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x3]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x4]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x5]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x6]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x7]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x8]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0x9]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xa]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xb]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xc]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xd]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xe]), " ",
                                          std::hex, std::setw(3), std::setfill('0'), static_cast<uint>(m_stack[0xf]), Logger::endl);

    TickResult tickRes;
    tickRes.displayUpdate = 0;
    tickRes.soundState = 0;

    chip8Logger.log<Logger::LogTrace>("Chip8: t_counter:", static_cast<uint>(m_tCounter), Logger::endl);

    if(++m_tCounter >= 10){
        m_tCounter = 0;
        if(m_dtReg){
            m_dtReg--;
        }
        if(m_stReg){
            m_stReg--;
            tickRes.soundState = 1;
        }
        else
            tickRes.soundState = 0;
    }

    uint16_t arith_res;
    #ifdef LITTLE_ENDIAN
        uint16_t op = (m_memory[m_programCounter] << 8) |m_memory[m_programCounter + 1];
    #else
        uint16_t op = *((uint16_t*)  (m_memory + m_programCounter));
    #endif
    chip8Logger.log<Logger::LogDebug>("Chip8: pc: ", std::hex, std::setw(4), std::setfill('0'), m_programCounter, Logger::endl);
    chip8Logger.log<Logger::LogDebug>("Chip8: op: ", std::hex, std::setw(4), std::setfill('0'), op, Logger::endl);
    switch(op & 0xf000){
        case 0x0000:
            switch(op & 0x0fff){
                case 0x0e0:
                    // CLS
                    memset(m_disp, 0, CHIP8_DISP_Y * (CHIP8_DISP_X / 8));
                    m_programCounter+=2;
                    tickRes.displayUpdate = 1;
                    break;
                case 0x0ee:
                    //RET
                    // if(m_stackPointer == 0xf){
                    //     std::stringstream err_msg_stream;
                    //     err_msg_stream << "Chip8: m_stack wrap, call m_stack empty, triggered by RET at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    //     throw err_msg_stream.str();
                    // }
                    m_programCounter = m_stack[++m_stackPointer];
                    m_programCounter += 2;
                    break;
                default:
                    std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
            }
            break;
        case 0x1000:
            // JMP nnn
            m_programCounter = CHIP8_NNN;
            break;
        case 0x2000:
            // CALL nnn
            // if(m_stackPointer == 0x0){
            //     std::stringstream err_msg_stream;
            //     err_msg_stream << "Chip8: m_stack wrap, call m_stack full, triggered by CALL at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
            //     throw err_msg_stream.str();
            // }
            m_stack[m_stackPointer--] = m_programCounter;
            m_programCounter = CHIP8_NNN;
            break;
        case 0x3000:
            // SE VX, KK
            if(CHIP8_VX == CHIP8_KK){
                m_programCounter+=2;
            }
            m_programCounter+=2;
            break;
        case 0x4000:
            //SNE VX, KK
            if(CHIP8_VX != CHIP8_KK){
                m_programCounter+=2;
            }
            m_programCounter+=2;
            break;
        case 0x5000:
            // SE VX, VY
            if(CHIP8_VX == CHIP8_VY){
                m_programCounter+=2;
            }
            m_programCounter+=2;
            break;
        case 0x6000:
            // LD VX, KK
            CHIP8_VX = CHIP8_KK;
            m_programCounter+=2;
            break;
        case 0x7000:
            // ADD VX, KK
            CHIP8_VX += CHIP8_KK;
            m_programCounter+=2;
            break;
        case 0x8000:
            switch(CHIP8_NIB){
                case 0:
                    // LD VX, VY
                    CHIP8_VX = CHIP8_VY;
                    m_programCounter+=2;
                    break;
                case 1:
                    // OR VX, VY
                    CHIP8_VX |= CHIP8_VY;
                    m_programCounter+=2;
                    break;
                case 2:
                    // AND VX, VY
                    CHIP8_VX &= CHIP8_VY;
                    m_programCounter+=2;
                    break;
                case 3:
                    // XOR VX, VY
                    CHIP8_VX ^= CHIP8_VY;
                    m_programCounter+=2;
                    break;
                case 4:
                    // ADD VX,
                    arith_res = CHIP8_VX + CHIP8_VY;
                    CHIP8_V(0xf) = (arith_res & 0x100) ? 1 : 0;
                    CHIP8_VX = arith_res & 0xff;
                    m_programCounter+=2;
                    break;
                case 5:
                    // SUB VX, VY
                    CHIP8_V(0xf) = (CHIP8_VX > CHIP8_VY)? 1 : 0;
                    CHIP8_VX -= CHIP8_VY;
                    m_programCounter+=2;
                    break;
                case 6:
                    //SHR VX, Vy
                    CHIP8_V(0xf) = CHIP8_VX & 0x1;
                    CHIP8_VX >>= 1;
                    m_programCounter+=2;
                    break;
                case 7:
                    //SUBN VX, VY
                    m_vRegs[0xf] = (CHIP8_VY > CHIP8_VX)? 1 : 0;
                    CHIP8_VX = CHIP8_VY - CHIP8_VX;
                    m_programCounter+=2;
                    break;
                case 0xe:
                    //SHL VX, VY
                    m_vRegs[0xf] = (CHIP8_VX & 0x80)? 1 : 0;
                    CHIP8_VX <<= 1;
                    m_programCounter+=2;
                    break;
                default:
                    std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
        }
            break;
        case 0x9000:
            // SNE VX, VY
            if(CHIP8_VX != CHIP8_VY){
                m_programCounter+=2;
            }
            m_programCounter+=2;
            break;
        case 0xa000:
            // LD I, NNN
            m_iReg = CHIP8_NNN;
            m_programCounter+=2;
            break;
        case 0xb000:
            // JMP V0, NNN
            m_programCounter = CHIP8_NNN + m_vRegs[0];
            m_programCounter+=2;
            break;
        case 0xc000:
            // RND VX, KK
            CHIP8_VX = m_dist(m_generator) & CHIP8_KK;
            m_programCounter+=2;
            break;
        case 0xd000:
            // DRW VX, VY, NIB
            for(int i = 0; i < CHIP8_NIB; i++){
                uint8_t sprite_byte =m_memory[m_iReg + i];
                uint8_t sprite_x = CHIP8_VX >> 3, sprite_y = CHIP8_VY + i;
                if(sprite_y > 0x1f){
                    sprite_y -= 0x20;
                }
                CHIP8_V(0xf) |= (m_disp[sprite_x + sprite_y * (CHIP8_DISP_X >> 3)] & (sprite_byte >> (CHIP8_VX & 0x7)))? 1 : 0;
                m_disp[sprite_x + sprite_y * (CHIP8_DISP_X >> 3)] ^= (sprite_byte >> (CHIP8_VX & 0x7));
                if(CHIP8_VX & 0x7){
                    if(sprite_x + 1 > 0x3f){
                        sprite_x -= 0x3f;
                    }
                        CHIP8_V(0xf) |= (m_disp[sprite_x + sprite_y * (CHIP8_DISP_X >> 3)] & (sprite_byte << (8 - (CHIP8_VX & 0x7))))? 1 : 0;
                        m_disp[sprite_x + sprite_y * (CHIP8_DISP_X >> 3)] ^= (sprite_byte << (8 - (CHIP8_VX & 0x7)) & 0xff);
                }
            }
            tickRes.displayUpdate = 1;
            m_programCounter+=2;
            break;
        case 0xe000:
            switch(CHIP8_KK){
                case 0x9e:
                    // SKP VX
                    if((m_keystates >> CHIP8_VX) & 0x1){
                        m_programCounter+=2;
                    }
                    m_programCounter+=2;
                    break;
                case 0xa1:
                    //SKNP VX
                    if(!((m_keystates >> CHIP8_VX) & 0x1)){
                        m_programCounter+=2;
                    }
                    m_programCounter+=2;
                    break;
                default:
                    std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
            }
            break;
        case 0xf000:
            switch(CHIP8_KK){
                case 0x07:
                    // LD VX, DT
                    CHIP8_VX = m_dtReg; 
                    m_programCounter+=2;
                    break;
                case 0x0a:
                    // LD VX, KP
                    if(m_keystates){
                        uint8_t i = 0;
                        while((m_keystates >> i++) & 0x1){
                            CHIP8_VX = i - 1;
                        }
                        m_programCounter+=2;
                    } 
                    break;
                case 0x15:
                    // LD DT, VX
                    m_dtReg = CHIP8_VX;
                    m_programCounter+=2;
                    break;
                case 0x18:
                    // LD ST, VX
                    m_stReg = CHIP8_VX;
                    if(m_stReg){
                        tickRes.soundState = 1;
                    }   
                    m_programCounter+=2;
                    break;
                case 0x1e:
                    // ADD I, VX
                    m_iReg += CHIP8_VX;
                    m_programCounter+=2;
                    break;
                case 0x29:
                    // LD I, Sprt(VX)
                    m_iReg = CHIP8_VX * 5;
                    m_programCounter+=2;
                    break;
                case 0x33:
                    // LD I, BCD(VX)
                    m_memory[m_iReg] = CHIP8_VX / 100;
                    m_memory[m_iReg +1] = (CHIP8_VX % 100) / 10;
                    m_memory[m_iReg + 2] = CHIP8_VX % 10;
                    m_programCounter+=2;
                    break;
                case 0x55:
                    // LD [I], VX
                    for(uint8_t i = 0; i <= CHIP8_OP_X; i++)
                       m_memory[m_iReg + i] = CHIP8_V(i);
                    m_programCounter+=2; 
                    break;
                case 0x65:
                    // LD VX, [I]
                    for(uint8_t i = 0; i <= CHIP8_OP_X; i++)
                        CHIP8_V(i) =m_memory[m_iReg + i]; 
                    m_programCounter+=2;
                    break;
                default:
                    std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
            }
            break;
        default:
                std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
    }

    return tickRes;
}

}
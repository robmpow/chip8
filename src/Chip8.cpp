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
#include <climits>

#include "Bitfield.hpp"
#include "Chip8.hpp"
#include "LoggerImpl.hpp"

namespace Chip8{

Chip8::Chip8(std::mt19937::result_type t_seed) : m_dist(0, 255), m_seed(t_seed) {
    reset(m_seed);
}

Chip8::Chip8(std::mt19937::result_type seed, const std::string& filePath) : m_dist(0, 255), m_seed(seed) {
    reset(m_seed);
    load(filePath);
}

Chip8::Chip8(std::mt19937::result_type seed, std::istream& inStream) : m_dist(0, 255), m_seed(seed){
    reset(m_seed);
    load(inStream);
}

void Chip8::reset(){
    reset(m_seed);
}

void Chip8::reset(std::mt19937::result_type seed) {
    std::copy(m_spriteTable.begin(), m_spriteTable.end(), m_memory.begin());
    std::fill(m_memory.begin() + CHIP8_SPRITE_TABLE_SIZE, m_memory.end(), 0);
    std::fill(m_stack.begin(), m_stack.end(), 0);
    std::fill(m_vRegs.begin(), m_vRegs.end(), 0);
    m_iReg = 0;
    m_dtReg = 0;
    m_stReg = 0;
    m_tCounter = 0;
    std::fill(std::begin(m_disp), std::end(m_disp), 0);
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
    inStream.read((char*) (m_memory.begin() + CHIP8_PROG_START_OFFSET), CHIP8_MAIN_MEM_SIZE - CHIP8_PROG_START_OFFSET);
}

void Chip8::updateKeystate(bool press_state, bool repeat, const Chip8Key& key){
    if(key != KEY_NULL)
        m_keystates = (m_keystates & ~(0x1 << key)) | (press_state << key & (0x1 << key));
}

void Chip8::CLS(){
    std::fill(m_disp.begin(), m_disp.end(), 0);
}

void Chip8::RET(){
    m_programCounter = m_stack[++m_stackPointer];
}

void Chip8::JMP(uint16_t t_nnn){
    m_programCounter = t_nnn;
}

void Chip8::CALL(uint16_t t_nnn){
    m_stack[m_stackPointer--] = m_programCounter;
    m_programCounter = t_nnn;
}

void Chip8::SE_IMM(uint8_t t_x, uint8_t t_kk){
    if(m_vRegs[t_x] == t_kk){
        m_programCounter += 2;
    }
}

void Chip8::SNE_IMM(uint8_t t_x, uint8_t t_kk){
    if(m_vRegs[t_x] != t_kk){
        m_programCounter += 2;
    }
}

void Chip8::SE_REG(uint8_t t_x, uint8_t t_y){
    if(m_vRegs[t_x] == m_vRegs[t_y]){
        m_programCounter += 2;
    }
}

void Chip8::LD_IMM(uint8_t t_x, uint8_t t_kk){
    m_vRegs[t_x] = t_kk;
}

void Chip8::ADD_IMM(uint8_t t_x, uint8_t t_kk){
    m_vRegs[t_x] += t_kk;
}

void Chip8::LD_REG(uint8_t t_x, uint8_t t_y){
    m_vRegs[t_x] = m_vRegs[t_y];
}

void Chip8::OR(uint8_t t_x, uint8_t t_y){
    m_vRegs[t_x] |= m_vRegs[t_y];
}

void Chip8::AND(uint8_t t_x, uint8_t t_y){
    m_vRegs[t_x] &= m_vRegs[t_y];
}

void Chip8::XOR(uint8_t t_x, uint8_t t_y){
    m_vRegs[t_x] ^= m_vRegs[t_y];
}

void Chip8::ADD_REG(uint8_t t_x, uint8_t t_y){
    m_vRegs[0xf] = (m_vRegs[t_x] > (UINT8_MAX - m_vRegs[t_y]))? 0x01 : 0x00;
    m_vRegs[t_x] += m_vRegs[t_y];
}

void Chip8::SUB_REG(uint8_t t_x, uint8_t t_y){
    m_vRegs[0xf] = static_cast<uint8_t>(m_vRegs[t_x] > m_vRegs[t_y]);
    m_vRegs[t_x] -= m_vRegs[t_y];
}

void Chip8::SHR(uint8_t t_x){
    m_vRegs[0xf] = m_vRegs[t_x] & 0x01;
    m_vRegs[t_x] >>= 1;
}

void Chip8::SUBN(uint8_t t_x, uint8_t t_y){
    m_vRegs[0xf] = static_cast<uint8_t>(m_vRegs[t_y] > m_vRegs[t_x]);
    m_vRegs[t_x] = m_vRegs[t_y] - m_vRegs[t_x];
}

void Chip8::SHL(uint8_t t_x){
    m_vRegs[0xf] = (m_vRegs[t_x] & 0x80)? 0x01 : 0x00;
    m_vRegs[t_x] <<= 1;
}

void Chip8::SNE_REG(uint8_t t_x, uint8_t t_y){
    if(m_vRegs[t_x] != m_vRegs[t_y]){
        m_programCounter += 2;
    }
}

void Chip8::LD_I(uint16_t t_nnn){
    m_iReg = t_nnn & 0x0fff;
}

void Chip8::JMP_REG(uint16_t t_nnn){
    m_programCounter = m_vRegs[0] + t_nnn;
}

void Chip8::RND(uint8_t t_x, uint8_t t_kk){
    m_vRegs[t_x] = m_dist(m_generator) & t_kk;
}

void Chip8::DRW(uint8_t t_x, uint8_t t_y, uint8_t t_n){
    m_vRegs[0xf] = 0x00;
    for(uint8_t spriteLine = 0; spriteLine < t_n; ++spriteLine){
        uint8_t spriteByte = m_memory[m_iReg + spriteLine];
        uint8_t dispX = m_vRegs[t_x] >> 3, dispY = m_vRegs[t_y] + spriteLine;

        if(dispY > 0x1f){
            dispY -= 0x20;
        }
        m_vRegs[0xf] |= (m_disp[dispX + dispY * (CHIP8_DISP_X >> 3)] & (spriteByte >> (m_vRegs[t_x] & 0x07)))? 0x01 : 0x00;
        m_disp[dispX + dispY * (CHIP8_DISP_X >> 3)] ^= (spriteByte >> (m_vRegs[t_x] & 0x07));
        if(m_vRegs[t_x] & 0x7){
            if(dispX + 1 > 0x3f){
                dispX -= 0x3f;
            }
            m_vRegs[0xf] |= (m_disp[dispX + 1 + dispY * (CHIP8_DISP_X >> 3)] & (spriteByte << (8 - (m_vRegs[t_x] & 0x07))))? 0x01 : 0x00;
            m_disp[dispX + 1 + dispY * (CHIP8_DISP_X >> 3)] ^= (spriteByte << (8 - (m_vRegs[t_x] & 0x07)));
        }
    }
}

void Chip8::SKP(uint8_t t_x){
    if(m_vRegs[t_x] < 0x10 && (m_keystates >> m_vRegs[t_x]) & 0x01){
        m_programCounter += 2;
    }
}

void Chip8::SKNP(uint8_t t_x){
    if(m_vRegs[t_x] < 0x10 && !((m_keystates >> m_vRegs[t_x]) & 0x01)){
        m_programCounter += 2;
    }
}

void Chip8::LD_VX_DT(uint8_t t_x){
    m_vRegs[t_x] = m_dtReg;
}

void Chip8::LD_KP(uint8_t t_x){
    if(m_keystates){
        uint8_t keyIndex = 0;
        while(m_keystates >> keyIndex){
            if((m_keystates >> keyIndex) & 0x01){
                m_vRegs[t_x] = keyIndex;
                m_programCounter += 2;
                break;
            }
            ++keyIndex;
        }
    }
}

void Chip8::LD_DT_VX(uint8_t t_x){
    m_dtReg = m_vRegs[t_x];
}

void Chip8::LD_ST(uint8_t t_x){
    m_stReg = m_vRegs[t_x];
}

void Chip8::ADD_I(uint8_t t_x){
    m_iReg += m_vRegs[t_x];
}

void Chip8::LD_SPRT(uint8_t t_x){
    m_iReg = 5 * m_vRegs[t_x];
}

void Chip8::LD_BCD(uint8_t t_x){
    m_memory[m_iReg] = m_vRegs[t_x] / 100;
    m_memory[m_iReg +1] = (m_vRegs[t_x] % 100) / 10;
    m_memory[m_iReg + 2] = m_vRegs[t_x] % 10;
}

void Chip8::LD_MEM(uint8_t t_x){
    for(uint8_t i = 0; i <= t_x; ++i){
        m_memory[m_iReg + i] = m_vRegs[i];
    }
}

void Chip8::LD_REGS(uint8_t t_x){
    for(uint8_t i = 0; i <= t_x; ++i){
        m_vRegs[i] = m_memory[m_iReg + i];
    }
}


TickResult Chip8::run_tick() {


    chip8Logger.log<Logger::LogTrace>("Chip8: key state: 0x", std::hex, std::setw(4), std::setfill('0'), m_keystates, Logger::endl);

    chip8Logger.log<Logger::LogTrace>("Chip8: vregs{0x0,0xf}: ", std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x0]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x1]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x2]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x3]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x4]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x5]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x6]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x7]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x8]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0x9]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xa]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xb]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xc]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xd]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xe]), " ",
                                         std::hex, std::setw(2), std::setfill('0'), static_cast<uint>(m_vRegs[0xf]), Logger::endl);
    
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

    #ifdef LITTLE_ENDIAN
        uint16_t op = (m_memory[m_programCounter] << 8) |m_memory[m_programCounter + 1];
    #else
        uint16_t op = *((uint16_t*)  (m_memory + m_programCounter));
    #endif

    chip8Logger.log<Logger::LogDebug>("Chip8: pc: ", std::hex, std::setw(4), std::setfill('0'), m_programCounter, Logger::endl);
    chip8Logger.log<Logger::LogDebug>("Chip8: op: ", std::hex, std::setw(4), std::setfill('0'), op, Logger::endl);

    #define OPX ((op >> 8) & 0x0f)
    #define OPY ((op >> 4) & 0x0f)
    #define KK  (op & 0x00ff)
    #define NNN (op & 0x0fff)
    #define NIB (op & 0x000f)

    switch(op & 0xf000){
        case 0x0000:
            switch(op & 0x0fff){
                case 0x0e0:
                    // CLS
                    CLS();
                    tickRes.displayUpdate = 1;
                    m_programCounter+=2;
                    break;
                case 0x0ee:
                    //RET
                    RET();
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
            JMP(NNN);
            break;
        case 0x2000:
            // CALL nnn
            CALL(NNN);
            break;
        case 0x3000:
            // SE VX, KK
            SE_IMM(OPX, KK);
            m_programCounter+=2;
            break;
        case 0x4000:
            //SNE VX, KK
            SNE_IMM(OPX, KK);
            m_programCounter+=2;
            break;
        case 0x5000:
            // SE VX, VY
            SE_REG(OPX, OPY);
            m_programCounter+=2;
            break;
        case 0x6000:
            // LD VX, KK
            LD_IMM(OPX, KK);
            m_programCounter+=2;
            break;
        case 0x7000:
            // ADD VX, KK
            ADD_IMM(OPX, KK);
            m_programCounter+=2;
            break;
        case 0x8000:
            switch(NIB){
                case 0:
                    // LD VX, VY
                    LD_REG(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 1:
                    // OR VX, VY
                    OR(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 2:
                    // AND VX, VY
                    AND(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 3:
                    // XOR VX, VY
                    XOR(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 4:
                    // ADD VX, VY
                    ADD_REG(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 5:
                    // SUB VX, VY
                    SUB_REG(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 6:
                    //SHR VX, Vy
                    SHR(OPX);
                    m_programCounter+=2;
                    break;
                case 7:
                    //SUBN VX, VY
                    SUBN(OPX, OPY);
                    m_programCounter+=2;
                    break;
                case 0xe:
                    //SHL VX, VY
                    SHL(OPX);
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
            SNE_REG(OPX, OPY);
            m_programCounter+=2;
            break;
        case 0xa000:
            // LD I, NNN
            LD_I(NNN);
            m_programCounter+=2;
            break;
        case 0xb000:
            // JMP V0, NNN
            JMP_REG(NNN);
            m_programCounter+=2;
            break;
        case 0xc000:
            // RND VX, KK
            RND(OPX, KK);
            m_programCounter+=2;
            break;
        case 0xd000:
            // DRW VX, VY, NIB
            DRW(OPX, OPY, NIB);
            tickRes.displayUpdate = 1;
            m_programCounter+=2;
            break;
        case 0xe000:
            switch(KK){
                case 0x9e:
                    // SKP VX
                    SKP(OPX);
                    m_programCounter+=2;
                    break;
                case 0xa1:
                    //SKNP VX
                    SKNP(OPX);
                    m_programCounter+=2;
                    break;
                default:
                    std::stringstream err_msg_stream;
                    err_msg_stream << "Chip8: Unknown opcode <0x" << std::hex << std::setfill('0') << std::setw(4) << op << "> at address <0x" << std::hex << std::setfill('0') << std::setw(4) << m_programCounter << ">.";
                    throw err_msg_stream.str();
            }
            break;
        case 0xf000:
            switch(KK){
                case 0x07:
                    // LD VX, DT
                    LD_VX_DT(OPX);
                    m_programCounter+=2;
                    break;
                case 0x0a:
                    // LD VX, KP
                    LD_KP(OPX);
                    break;
                case 0x15:
                    // LD DT, VX
                    LD_DT_VX(OPX);
                    m_programCounter+=2;
                    break;
                case 0x18:
                    // LD ST, VX
                    LD_ST(OPX);
                    m_programCounter+=2;
                    break;
                case 0x1e:
                    // ADD I, VX
                    ADD_I(OPX);
                    m_programCounter+=2;
                    break;
                case 0x29:
                    // LD I, Sprt(VX)
                    LD_SPRT(OPX);
                    m_programCounter+=2;
                    break;
                case 0x33:
                    // LD I, BCD(VX)
                    LD_BCD(OPX);
                    m_programCounter+=2;
                    break;
                case 0x55:
                    // LD [I], VX
                    LD_MEM(OPX);
                    m_programCounter+=2; 
                    break;
                case 0x65:
                    // LD VX, [I]
                    LD_REGS(OPX);
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

    #undef OPX
    #undef OPY
    #undef KK 
    #undef NNN
    #undef NIB
}

}
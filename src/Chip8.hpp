//
// Created by rmpowell on 10/3/18.
//

#ifndef CHIP8_INTERPRETER_H
#define CHIP8_INTERPRETER_H

#include <cstdint>
#include <string>
#include <random>

#include "Bitfield.hpp"

#define CHIP8_SPRITE_TABLE_SIZE   0x0050
#define CHIP8_MAIN_MEM_SIZE       0x1000
#define CHIP8_PROG_START_OFFSET   0x0200
#define CHIP8_NUM_V_REG           0x0010
#define CHIP8_STACK_SIZE          0x0010
#define CHIP8_DISP_SIZE           0x0100

#define CHIP8_DISP_X              0x0040
#define CHIP8_DISP_Y              0x0020

#define CHIP8_TICK_PERIOD_USEC 2000 

namespace Chip8{

union TickResult{
    Bitfield::Bitfield<uint8_t, 0, 1> displayUpdate;
    Bitfield::Bitfield<uint8_t, 1, 1> soundState;
    Bitfield::Bitfield<uint8_t, 0, 2> all;
};

enum Chip8Key{
    KEY_NULL = -1,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
};

class Chip8 {
protected:

    std::mt19937 m_generator;
    std::uniform_int_distribution<> m_dist;

    const std::array<uint8_t, CHIP8_SPRITE_TABLE_SIZE>m_spriteTable = {0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
                                                                       0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
                                                                       0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
                                                                       0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
                                                                       0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
                                                                       0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
                                                                       0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
                                                                       0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
                                                                       0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
                                                                       0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
                                                                       0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
                                                                       0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
                                                                       0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
                                                                       0xE0, 0x90, 0x90, 0x90, 0xE0, /* E */
                                                                       0xF0, 0x80, 0xF0, 0x80, 0x80, /* F */ };
    std::mt19937::result_type m_seed;
    std::array<uint8_t, CHIP8_MAIN_MEM_SIZE> m_memory;
    std::array<uint8_t, CHIP8_NUM_V_REG> m_vRegs;
    std::array<uint16_t, CHIP8_STACK_SIZE> m_stack;
    std::array<uint8_t, CHIP8_DISP_SIZE> m_disp;
    uint16_t m_iReg;
    uint8_t m_dtReg;
    uint8_t m_stReg;
    uint8_t m_stackPointer : 4;
    uint16_t m_programCounter;
    uint8_t m_tCounter;
    uint16_t m_keystates;

    // Opcode functions
    void CLS();
    void RET();
    void JMP(uint16_t t_nnn);
    void CALL(uint16_t t_nnn);
    void SE_IMM(uint8_t t_x, uint8_t t_kk);
    void SNE_IMM(uint8_t t_x, uint8_t t_kk);
    void SE_REG(uint8_t t_x, uint8_t t_y);
    void LD_IMM(uint8_t t_x, uint8_t t_kk);
    void ADD_IMM(uint8_t t_x, uint8_t t_kk);
    void LD_REG(uint8_t t_x, uint8_t t_y);
    void OR(uint8_t t_x, uint8_t t_y);
    void AND(uint8_t t_x, uint8_t t_y);
    void XOR(uint8_t t_x, uint8_t t_y);
    void ADD_REG(uint8_t t_x, uint8_t t_y);
    void SUB_REG(uint8_t t_x, uint8_t t_y);
    void SHR(uint8_t t_x);
    void SUBN(uint8_t t_x, uint8_t t_y);
    void SHL(uint8_t t_x);
    void SNE_REG(uint8_t t_x, uint8_t t_y);
    void LD_I(uint16_t t_nnn);
    void JMP_REG(uint16_t t_nnn);
    void RND(uint8_t t_x, uint8_t t_kk);
    void DRW(uint8_t t_x, uint8_t t_y, uint8_t t_n);
    void SKP(uint8_t t_x);
    void SKNP(uint8_t t_x);
    void LD_VX_DT(uint8_t t_x);
    void LD_KP(uint8_t t_x);
    void LD_DT_VX(uint8_t t_x);
    void LD_ST(uint8_t t_x);
    void ADD_I(uint8_t t_x);
    void LD_SPRT(uint8_t t_x);
    void LD_BCD(uint8_t t_x);
    void LD_MEM(uint8_t t_x);
    void LD_REGS(uint8_t t_x);

public:
    Chip8(std::mt19937::result_type t_seed);
    Chip8(std::mt19937::result_type t_seed, const std::string& t_rom);
    Chip8(std::mt19937::result_type t_seed, std::istream& t_inStream);

    void reset();
    void reset(std::mt19937::result_type t_seed);
    void load(const std::string& t_filePath);
    void load(std::istream& t_iStream);
    TickResult run_tick();

    void updateKeystate(bool t_pressState, bool t_repeat, const Chip8Key& t_key);

    std::array<uint8_t, ((CHIP8_DISP_X >> 3) * CHIP8_DISP_Y)>::const_iterator getDisplay(){
        return m_disp.begin();
    }
};

} // namespace Chip8

#endif // CHIP8_INTERPRETER_H
 
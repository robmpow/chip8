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

#define CHIP8_V(ind)          m_vRegs[ind]
#define CHIP8_OP_X            ((op >> 8) & 0xF)
#define CHIP8_OP_Y            ((op >> 4) & 0xF)
#define CHIP8_VX              m_vRegs[(op >> 8) & 0xF]
#define CHIP8_VY              m_vRegs[(op >> 4) & 0xF]
#define CHIP8_NNN             (op & 0x0FFF)
#define CHIP8_KK              static_cast<uint8_t>(op & 0x00FF)
#define CHIP8_NIB             (op & 0xF)

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
private:

    std::mt19937 m_generator;
    std::uniform_int_distribution<> m_dist;

    const uint8_t m_sprite_table[CHIP8_SPRITE_TABLE_SIZE] = {   0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
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
    uint m_seed;

    uint8_t m_memory[CHIP8_MAIN_MEM_SIZE];
    uint8_t m_vRegs[CHIP8_NUM_V_REG];
    uint16_t m_stack[CHIP8_STACK_SIZE];
    uint8_t m_disp[CHIP8_DISP_SIZE];
    uint16_t m_iReg;
    uint8_t m_dtReg;
    uint8_t m_stReg;

    uint8_t m_stackPointer : 4;
    uint16_t m_programCounter;

    uint8_t m_tCounter;

    uint16_t m_keystates;
    

public:
    Chip8(uint seed, const std::string& file_name);
    Chip8(uint seed, std::istream& inStream);

    void reset();
    void reset(uint seed);
    void load(const std::string& file_path);
    void load(std::istream& in_stream);
    TickResult run_tick();

    void updateKeystate(bool press_state, bool repeat, const Chip8Key& key);

    uint8_t* getDisplay(){
        return m_disp;
    }
};
}

#endif // CHIP8_INTERPRETER_H
 
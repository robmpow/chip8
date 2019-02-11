//
// Created by rmpowell on 10/3/18.
//
#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <string>
#include <random>

#include "bitfield.h"

#define SPRITE_TABLE_SIZE   0x0050
#define MAIN_MEM_SIZE       0x1000
#define PROG_START_OFFSET   0x0200
#define NUM_V_REG           0x0010
#define STACK_SIZE          0x0010
#define DISP_SIZE           0x0100

#define DISP_X              0x0040
#define DISP_Y              0x0020

#define CHIP8_TICK_PERIOD_USEC 2000 

#define V(ind)          v_regs[ind]
#define OP_X            ((op >> 8) & 0xF)
#define OP_Y            ((op >> 4) & 0xF)
#define VX              v_regs[(op >> 8) & 0xF]
#define VY              v_regs[(op >> 4) & 0xF]
#define NNN             (op & 0x0FFF)
#define KK              static_cast<uint8_t>(op & 0x00FF)
#define NIB             (op & 0xF)

union tick_result{
    bitfield<uint8_t, 0, 1> display_update;
    bitfield<uint8_t, 1, 1> sound_state;
    bitfield<uint8_t, 0, 2> all;
};

enum chip8_key{
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

class chip8 {

private:

    std::mt19937 generator;
    std::uniform_int_distribution<> dist;

    const uint8_t sprite_table[SPRITE_TABLE_SIZE] = {   0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
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
    uint seed;

    uint8_t memory[MAIN_MEM_SIZE];
    uint8_t v_regs[NUM_V_REG];
    uint16_t stack[STACK_SIZE];
    uint8_t disp[DISP_SIZE];
    uint16_t i_reg;
    uint8_t dt_reg;
    uint8_t st_reg;

    uint8_t stack_pointer : 4;
    uint16_t program_counter;

    uint8_t t_counter;

    uint16_t key_states;
    

public:
    chip8(uint seed, std::string file_name);
    chip8(uint seed, std::istream& file_stream);

    void reset();
    void reset(uint seed);
    void load(std::string file_path);
    void load(std::istream& in_stream);
    tick_result run_tick();

    void update_keystate(bool press_state, bool repeat, chip8_key key);

    uint8_t* getDisplay(){
        return disp;
    }
};


#endif //CHIP8_H
 
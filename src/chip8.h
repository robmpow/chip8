//
// Created by rmpowell on 10/3/18.
//

#include <cstdint>
#include <string>

#ifndef CHIP8_H
#define CHIP8_H

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

#define MAIN_MEM_OFFSET     0X0000
#define SPRITE_TABLE_SIZE   0x0050
#define PROG_START_OFFSET   0x0200
#define MAIN_MEM_SIZE       0x1000
#define V_REG_OFFSET        0x1000
#define NUM_V_REG           0x0010
#define I_REG_OFFSET        0x1010
#define DT_REG_OFFSET       0x1012
#define ST_REG_OFFSET       0x1013
#define STACK_OFFSET        0x1014
#define STACK_SIZE          0x0020
#define DISP_OFFSET         0x1034
#define DISP_SIZE           0x0100

#define DISP_X              0x0040
#define DISP_Y              0x0020

#define TOTAL_MEM_SIZE      0x1134

#define V(ind)          v_regs[ind]
#define Vx              v_regs[op >> 2 & 0xF]
#define Vy              v_regs[op >> 1 & 0xF]
#define nnn             static_cast<uint8_t>(op & 0x0FFF)
#define kk              static_cast<uint8_t>(op & 0x00FF)
#define nib             (op & 0xF)

class chip8io{
public:
    virtual void render();
    virtual void poll_input();
};

class chip8 {

private:
    const uint8_t sprite_table[SPRITE_TABLE_SIZE] = {   0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
                                                        0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
                                                        0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
                                                        0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
                                                        0x90, 0x90, 0x10, 0xF0, 0xF0, /* 4 */
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

    uint8_t* memory;
    uint8_t* v_regs;
    uint16_t* stack;
    uint8_t* disp;
    uint16_t* i_reg;
    uint8_t* dt_reg;
    uint8_t* st_reg;

    int8_t stack_pointer;
    uint16_t program_counter;

public:
    chip8(uint seed, chip8io* io);
    ~chip8();

    void reset();
    void load(std::string file_path);
    void run_tick();

private:
    void write(uint16_t address, uint8_t value);
    uint8_t read(uint16_t address);
};


#endif //CHIP8_H
 
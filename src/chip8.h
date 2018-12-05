//
// Created by rmpowell on 10/3/18.
//

#include <cstdint>
#include <string>

#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

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
 * |_____ 0x1016 _____|
 * |      Stack       |
 * |_____ 0x1036 _____|
 * |  Display Bitmap  |
 * |_____ 0x1836 _____|
 */
#define MEM_START           0X0000
#define SPRITE_TABLE_SIZE   0x0000
#define PROG_START          0x0200
#define MEM_SIZE            0x0FFF
#define REGISTERS_START     0x1000
#define NUM_REGISTERS       0x0010
#define REG_I               0x1010
#define REG_T               0x1012
#define STACK_START         0x1016
#define STACK_SIZE          0x000F
#define DISPLAY_START       0x1036
#define DISPLAY_X           0x0010
#define DISPLAY_Y           0x0020

#define TOTAL_MEM_SIZE      0x1836

#define V(ind)          registers[ind]
#define Vx              registers[op >> 2 & 0xF]
#define Vy              registers[op >> 1 & 0xF]
#define nnn             static_cast<uint8_t>(op & 0x0FFF)
#define kk              static_cast<uint8_t>(op & 0x00FF)
#define nib             (op & 0xF)

class chip8io{
public:
    virtual void memory_allocate(uint8_t** memory);
    virtual void render();
    virtual bool poll_input();
    virtual bool wait_input();
    virtual void memory_deallocate(uint8_t** memory);
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
    uint8_t* registers;
    uint16_t* stack;
    uint8_t* display;
    uint16_t* register_i;
    uint8_t* register_dt;
    uint8_t* register_st;

    int8_t stack_pointer;
    uint16_t program_counter;

public:
    chip8(uint seed, chip8io io);
    ~chip8();

    void reset();
    void load(std::string file_path);
    void run_tick();

private:
    void write(uint16_t address, uint8_t value);
    uint8_t read(uint16_t address);
};


#endif //CHIP8_CHIP8_H

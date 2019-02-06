#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Chip8
#include <boost/test/unit_test.hpp>
#include <boost/cstdint.hpp>

#ifndef NUM_FUZZ_TESTS
#define NUM_FUZZ_TESTS 255
#endif

#ifdef NO_SEED
#define SEED_RAND() srand(1)
#else
#define SEED_RAND() srand(time(NULL))
#endif

#include <iostream>
#include <iomanip>
#include <random>
#include <ctime>
#include <random>
#include <ctime>

#ifdef NO_ERROR
#define TEST_ASSERT BOOST_CHECK_MESSAGE
#else
#define TEST_ASSERT BOOST_REQUIRE_MESSAGE
#endif

#define RUN_TICK() try{ (void) ch8.run_tick();}catch(std::string err_msg){TEST_ASSERT(false, err_msg);}

// Hacky way to expose private members of chip8 class
#define private public
#include "../src/chip8.h"
#undef private

#define AS_HEX(width, value) std::hex << std::setfill('0') << std::setw(width) << (uint)(value)

struct membuff : std::streambuf{
    membuff(char* beg, char* end){
        this->setg(beg, beg, end);
    }
};

BOOST_AUTO_TEST_CASE (chip8_test_init){

    char test_load[MAIN_MEM_SIZE - PROG_START_OFFSET];

    SEED_RAND();

    for(uint i = 0; i < sizeof(test_load); i++){
        test_load[i] = (char) (rand() % 0xFF);
    }

    membuff test_prog_stream((char*) test_load, (char*) (test_load + sizeof(test_load)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(0, test_prog_istream);

    TEST_ASSERT(memcmp(test_load, ch8.memory + PROG_START_OFFSET, sizeof(test_load)) == 0, "Init test fail; memory contents do not match loaded program.");
}

BOOST_AUTO_TEST_CASE (chip8_test_LD){

    const uint8_t LD_V_test_prog[] = {    0x60, 0xF0, // [0x200] LD V0 = 0xF0
                                                0x61, 0xF1, // [0x202] LD V1 = 0xF1
                                                0x62, 0xF2, // [0x204] LD V1 = 0xF2
                                                0x63, 0xF3, // [0x206] LD V1 = 0xF3
                                                0x64, 0xF4, // [0x208] LD V1 = 0xF4
                                                0x65, 0xF5, // [0x20A] LD V1 = 0xF5
                                                0x66, 0xF6, // [0x20C] LD V1 = 0xF6
                                                0x67, 0xF7, // [0x20E] LD V1 = 0xF7
                                                0x68, 0xF8, // [0x210] LD V1 = 0xF8
                                                0x69, 0xF9, // [0x212] LD V1 = 0xF9
                                                0x6A, 0xFA, // [0x214] LD V1 = 0xFA
                                                0x6B, 0xFB, // [0x216] LD V1 = 0xFB
                                                0x6C, 0xFC, // [0x218] LD V1 = 0xFC
                                                0x6D, 0xFD, // [0x220] LD V1 = 0xFD
                                                0x6E, 0xFE, // [0x222] LD V1 = 0xFE
                                                0x6F, 0xFF, // [0x224] LD V1 = 0xFF
                                                0x80, 0x80, // [0x226] LD V0 = V8
                                                0x81, 0x90, // [0x228] LD V1 = V9
                                                0x82, 0xA0, // [0x22A] LD V2 = VA
                                                0x83, 0xB0, // [0x22C] LD V3 = VB
                                                0x84, 0xC0, // [0x22E] LD V4 = VC
                                                0x85, 0xD0, // [0x230] LD V5 = VD
                                                0x86, 0xE0, // [0x232] LD V6 = VE
                                                0x87, 0xF0, // [0x234] LD V7 = VF
                                                0x00,}; 

    membuff test_prog_stream((char*) LD_V_test_prog, (char*) (LD_V_test_prog + sizeof(LD_V_test_prog)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(0, test_prog_istream);

    for(int count = 0; count < 0x10; count++){
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[count] == count + 0xF0, "Load test fail for register V(0x" << count << "); Expected: <0X" << AS_HEX(2, count + 0xF0) << ">; Actual: <0x" << AS_HEX(2, ch8.v_regs[count]) << ">");
    }

    for(int count = 0; count < 0x08; count++){
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[count] == ch8.v_regs[count + 8], "Load test fail for register V(0x" << count << "); Expected: <0X" << AS_HEX(2, ch8.v_regs[count + 8]) << ">; Actual: <0x" << AS_HEX(2, ch8.v_regs[count]) << ">");
    }

}

BOOST_AUTO_TEST_CASE(chip8_test_dt_st){

    uint8_t DT_ST_test_prog[] = {   0x60, 0x00, // [0x200] LD V(rand_reg0) = rand_imm0
                                    0x60, 0x00, // [0x202] LD V(rand_reg1) = rand_imm1
                                    0xF0, 0x15, // [0x204] LD DT = V(rand_reg0)
                                    0xF0, 0x18, // [0x206] LD ST = V(rand_reg1)
                                    0xF0, 0x07, // [0x208] LD V(rand_reg1) = DT
                                    0x30, 0x00, // [0x20A] SE V(rand_reg1) == 0x00
                                    0x12, 0x08, // [0x20C] JMP 0x208
                                    0x00, 0x00, // [0x20E] NOP
    };

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xF;
        while((rand_reg1 = rand() % 0xF) == rand_reg0);
        rand_imm1 = rand() & 0xFF;
        while((rand_imm0 = rand() & 0xFF) < rand_imm1);

        DT_ST_test_prog[0x00] = 0x60 | rand_reg0;   // [0x200] LD V(rand_reg0) = rand_imm0
        DT_ST_test_prog[0x01] = rand_imm0;          // [0x201] LD V(rand_reg0) = rand_imm0
        DT_ST_test_prog[0x02] = 0x60 | rand_reg1;   // [0x202] LD V(rand_reg1) = rand_imm1
        DT_ST_test_prog[0x03] = rand_imm1;          // [0x203] LD V(rand_reg1) = rand_imm1
        DT_ST_test_prog[0x04] = 0xF0 | rand_reg0;   // [0x204] LD DT = V(rand_reg0)
        DT_ST_test_prog[0x06] = 0xF0 | rand_reg1;   // [0x206] LD ST = V(rand_reg1)
        DT_ST_test_prog[0x08] = 0xF0 | rand_reg0;   // [0x208] LD V(rand_reg0) = DT
        DT_ST_test_prog[0x0A] = 0x30 | rand_reg1;   // [0x20A] SE V(rand_reg0) == 0x00

        membuff test_prog_stream((char*) DT_ST_test_prog, (char*) (DT_ST_test_prog + sizeof(DT_ST_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(0, test_prog_istream);

        // LD V(rand_reg0) = rand_imm0
        // LD V(rand_reg1) = rand_imm1
        RUN_TICK();
        RUN_TICK();

        // LD DT = V(rand_reg0)<rand_imm0>
        RUN_TICK();
        TEST_ASSERT(ch8.dt_reg == rand_imm0, "Load test case failed DT = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">, incorrect loaded value; Expected DT<0x" << AS_HEX(2, rand_imm0) << ">; Actual: DT<0x" << AS_HEX(2, ch8.dt_reg) << ">");

        // LD ST = V(rand_reg1)<rand_imm1>
        RUN_TICK();
        TEST_ASSERT(ch8.st_reg == rand_imm1, "Load test case failed ST = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">, incorrect loaded value; Expected ST<0x" << AS_HEX(2, rand_imm1) << ">; Actual: ST<0x" << AS_HEX(2, ch8.st_reg) << ">");

        int ticks = 4;
        while(ticks < 10 * rand_imm0){
            // LD v(rand_reg0) = DT
            RUN_TICK();
            TEST_ASSERT(ch8.v_regs[rand_reg0] == ch8.dt_reg, "Load test case failed for V(0x" << AS_HEX(1, rand_reg0) << ") = DT<0x" << AS_HEX(2, ch8.dt_reg) << ">; Expected V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, ch8.dt_reg) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, ch8.v_regs[rand_reg0]) << ">");
            
            RUN_TICK();
            RUN_TICK();

            ticks+=3;

            TEST_ASSERT(ch8.dt_reg == ((rand_imm0 - ticks / 10 > 0)? rand_imm0 - ticks / 10 : 0), "DT counter test case failed; Expected DT<0x" << AS_HEX(2, ((rand_imm0 - ticks / 10) > 0)? rand_imm0 - ticks / 10 : 0) << ">; Actual DT<0x" << AS_HEX(2, ch8.dt_reg) << ">");
        }
    }
}

BOOST_AUTO_TEST_CASE (chip8_test_LD_i){
    const uint8_t LD_i_test_prog[] = {    0x60, 0x00, // [0x200] LD V0 = 0x00
                                                0xF0, 0x29, // [0x202] LD I = Addr of sprite of the value of V0
                                                0x61, 0x01, // [0x204] LD V1 = 0x01
                                                0xF1, 0x29, // [0x206] LD I = Addr of sprite of the value of V1
                                                0x62, 0x02, // [0x208] LD V2 = 0x02
                                                0xF2, 0x29, // [0x20A] LD I = Addr of sprite of the value of V2
                                                0x63, 0x03, // [0x20C] LD V3 = 0x03
                                                0xF3, 0x29, // [0x20E] LD I = Addr of sprite of the value of V3
                                                0x64, 0x04, // [0x210] LD V4 = 0x04
                                                0xF4, 0x29, // [0x212] LD I = Addr of sprite of the value of V4
                                                0x65, 0x05, // [0x214] LD V5 = 0x05
                                                0xF5, 0x29, // [0x216] LD I = Addr of sprite of the value of V5
                                                0x66, 0x06, // [0x218] LD V6 = 0x06
                                                0xF6, 0x29, // [0x21A] LD I = Addr of sprite of the value of V6
                                                0x67, 0x07, // [0x21B] LD V7 = 0x07
                                                0xF7, 0x29, // [0x21E] LD I = Addr of sprite of the value of V7
                                                0x68, 0x08, // [0x220] LD V8 = 0x08
                                                0xF8, 0x29, // [0x222] LD I = Addr of sprite of the value of V8
                                                0x69, 0x09, // [0x224] LD V9 = 0x09
                                                0xF9, 0x29, // [0x226] LD I = Addr of sprite of the value of V9
                                                0x6A, 0x0A, // [0x228] LD VA = 0x0A
                                                0xFA, 0x29, // [0x22A] LD I = Addr of sprite of the value of VA
                                                0x6B, 0x0B, // [0x22C] LD VB = 0x0B
                                                0xFB, 0x29, // [0x22E] LD I = Addr of sprite of the value of VB
                                                0x6C, 0x0C, // [0x230] LD VC = 0x0C
                                                0xFC, 0x29, // [0x232] LD I = Addr of sprite of the value of VC
                                                0x6D, 0x0D, // [0x234] LD VD = 0x0D
                                                0xFD, 0x29, // [0x236] LD I = Addr of sprite of the value of VD
                                                0x6E, 0x0E, // [0x238] LD VE = 0x0E
                                                0xFE, 0x29, // [0x23A] LD I = Addr of sprite of the value of VE
                                                0x6F, 0x0F, // [0x23B] LD VF = 0x0F
                                                0xFF, 0x29, // [0x23C] LD I = Addr of sprite of the value of VF
                                                0xAF, 0xF0, // [0x23E] LD I = 0xFF0
                                                0x60, 0x9D, // [0x240] LD V0 = 0x9D (157)
                                                0xF0, 0x33, // [0x242] LD BCD representation of V0 into memory[I], memory[I+1], memory[I+2]
    };
                                    
    membuff test_prog_stream((char*) LD_i_test_prog, (char*) (LD_i_test_prog + sizeof(LD_i_test_prog)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(0, test_prog_istream);

    // Sprite digit repr
    for(int count = 0; count < 0x10; count++){
        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.i_reg == 5 * count, "Load test case failed for sprite addr load of V(0x" << count << ") = <0x" << AS_HEX(2, ch8.v_regs[count]) << "> into I register; Expected <0x" << AS_HEX(4, 5 * count) << ">; Actual <0x" << AS_HEX(4, ch8.i_reg) << ">");
    }

    // LD addr into I
    RUN_TICK();
    TEST_ASSERT(ch8.i_reg == 0xFF0, "Load test case failed for register I; Expected: <0x" << AS_HEX(4, 0xFF0) << ">; Actual: <0x" << AS_HEX(4, ch8.i_reg) << ">");

    // LD BCD repr of V0 into memory[I, I+1, I+2]
    RUN_TICK();
    RUN_TICK();
    const char expected_bcd[] = {1, 5, 7};
    TEST_ASSERT(memcmp(expected_bcd, ch8.memory + 0x0FF0, 3) == 0, "Load teast failed for BCD repr load of V(0) = <0x9D> (157); Expected: {1, 5, 7}; Actual: {" << uint(ch8.memory[ch8.i_reg]) << ", " << uint(ch8.memory[ch8.i_reg + 1]) << ", " << uint(ch8.memory[ch8.i_reg + 2]) << "}");
}

BOOST_AUTO_TEST_CASE (chip8_test_ADD){

    uint8_t ADD_test_prog[12];

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xF;
        while((rand_reg1 = rand() % 0xF) == rand_reg0);
        rand_imm0 = rand() % 0x7F;
        rand_imm1 = 0x80 + rand() % 0x7F;

        ADD_test_prog[0] = 0x60 | rand_reg0;        // [0x200] LD V(rand_reg0) = rand_imm0
        ADD_test_prog[1] = rand_imm0;               // [0x201] LD V(rand_reg0) = rand_imm0
        ADD_test_prog[2] = 0x70 | rand_reg0;        // [0x202] ADD V(rand_reg0) += rand_imm0
        ADD_test_prog[3] = rand_imm0;               // [0x203] ADD V(rand_reg0) += rand_imm0
        ADD_test_prog[4] = 0x60 | rand_reg1;        // [0x204] LD V(rand_reg1) = rand_imm1
        ADD_test_prog[5] = rand_imm1;               // [0x205] LD V(rand_reg1) = rand_imm1
        ADD_test_prog[6] = 0x80 | rand_reg1;        // [0x206] ADD V(rand_reg1) += V(rand_reg1) **OVERFLOW**
        ADD_test_prog[7] = (rand_reg1 << 4) | 0x04; // [0x207] ADD V(rand_reg1) += V(rand_reg1) **OVERFLOW**
        ADD_test_prog[8] = 0x60 | rand_reg0;        // [0x208] LD V(rand_reg1) = rand_imm1
        ADD_test_prog[9] = rand_imm0;               // [0x209] LD V(rand_reg1) = rand_imm1
        ADD_test_prog[10] = 0x80 | rand_reg0;       // [0x20A] ADD V(rand_reg0) += V(rand_reg0) **NO OVERFLOW**
        ADD_test_prog[11] = (rand_reg0 << 4) | 0x04;// [0x20B] ADD V(rand_reg0) += V(rand_reg0) **NO OVERFLOW**

        membuff test_prog_stream((char*) ADD_test_prog, (char*) (ADD_test_prog + sizeof(ADD_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(0, test_prog_istream);

        RUN_TICK();
        RUN_TICK();

        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 + rand_imm0) & 0xFF), "ADD imm test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0)<< ")<0x" << AS_HEX(2, (rand_imm0 + rand_imm0) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0)<< ")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 + rand_imm1) & 0xFF), "ADD reg test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> + <0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1)<< ")<0x" << AS_HEX(2, (rand_imm1 + rand_imm1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1)<< ")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x01, "ADD reg test case failed, incorrect carry flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> + <0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 + rand_imm0) & 0xFF), "ADD reg test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: <0x" << AS_HEX(2, (rand_imm0 + rand_imm0) & 0xFF) << ">; Actual <0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x00, "ADD reg test case failed, incorrect carry flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

    }
}

BOOST_AUTO_TEST_CASE (chip8_test_SUB){

    uint8_t SUB_test_prog[18]; 

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xF;
        while((rand_reg1 = rand() % 0xF) == rand_reg0);
        rand_imm0 = rand() % 0x7F;
        rand_imm1 = 0x80 + rand() % 0x7F;

        SUB_test_prog[0x00] = 0x60 | rand_reg0;       // [0x200] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x01] = rand_imm0;              // [0x201] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x02] = 0x60 | rand_reg1;       // [0x202] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x03] = rand_imm1;              // [0x203] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x04] = 0x80 | rand_reg1;       // [0x204] SUB V(rand_reg1) = V(rand_reg1) - V(rand_reg0) **NO BORROW**
        SUB_test_prog[0x05] = (rand_reg0 << 4) | 0x5; // [0x205] SUB V(rand_reg1) = V(rand_reg1) - V(rand_reg0) **NO BORROW**
        SUB_test_prog[0x06] = 0x60 | rand_reg1;       // [0x206] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x07] = rand_imm1;              // [0x207] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x08] = 0x80 | rand_reg0;       // [0x208] SUB V(rand_reg0) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x09] = (rand_reg1 << 4) | 0x5; // [0x209] SUB V(rand_reg0) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x0A] = 0x60 | rand_reg0;       // [0x20A] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x0B] = rand_imm0;              // [0x20B] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x0C] = 0x80 | rand_reg1;       // [0x20C] SUBN V(rand_reg1) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x0D] = (rand_reg0 << 4) | 0x7; // [0x20D] SUBN V(rand_reg1) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x0E] = 0x60 | rand_reg1;       // [0x20E] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x0F] = rand_imm1;              // [0x20F] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x10] = 0x80 | rand_reg0;       // [0x210] SUBN V(rand_reg0) = V(rand_reg1) - V(rand_reg0) **NO BORROW**
        SUB_test_prog[0x11] = (rand_reg1 << 4) | 0x7; // [0x211] SUBN V(rand_reg0) = V(rand_reg1) - V(rand_reg0) **NO BORROW**


        membuff test_prog_stream((char*) SUB_test_prog, (char*) (SUB_test_prog + sizeof(SUB_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        RUN_TICK();
        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 - rand_imm0) & 0xFF), "SUB test case failed, incorrect SUB result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 - rand_imm0) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x01, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 - rand_imm1) & 0xFF), "SUB test case failed, incorrect SUB result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 - rand_imm1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x00, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");


        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm0 - rand_imm1) & 0xFF), "SUB test case failed, incorrect SUBN result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm0 - rand_imm1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x00, "SUBN test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm1 - rand_imm0) & 0xFF), "SUB test case failed, incorrect SUBN result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm1 - rand_imm0) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x01, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

    }

}

BOOST_AUTO_TEST_CASE(chip8_test_JMP){

    uint8_t JMP_test_prog[] = { 0x12, 0x06, // [0x200] JMP 0x206
                                0x00, 0x00, // [0x202] NOP
                                0x00, 0x00, // [0x204] NOP
                                0x60, 0x00, // [0x206] LD V(rand_reg0) = rand_imm0
                                0x60, 0x00, // [0x208] LD V(rand_reg1) = rand_imm1
                                0x60, 0x00, // [0x20A] LD V(rand_reg2) = rand_imm0
                                0x30, 0x00, // [0x20C] SE V(rand_reg2) == rand_imm1     **NO SKIP**
                                0x30, 0x00, // [0x20E] SE V(rand_reg0) == rand_imm0     **SKIP**
                                0x00, 0x00, // [0x210] NOP
                                0x40, 0x00, // [0x212] SNE V(rand_reg0) != rand_imm0     **NO SKIP**
                                0x40, 0x00, // [0x214] SNE V(rand_reg1) != rand_imm0    **SKIP**
                                0x00, 0x00, // [0x216] NOP
                                0x90, 0x00, // [0x218] SNE V(rand_reg0) != V(rand_reg2)   **NO SKIP**
                                0x90, 0x00, // [0x21A] SNE V(rand_reg2) != V(rand_reg1) ** SKIP**
                                0x00, 0x00, // [0x21C] NOP
                                0x50, 0x00, // [0x21E] SE V(rand_reg0) == V(rand_reg1)  **NO SKIP**
                                0x50, 0x00, // [0x220] SE V(rand_reg2) == V(rand_reg0)  **SKIP** 
                                0x00, 0x00, // [0x222] NOP
                                0x60, 0x00, // [0x224] LD V(0) = 0 
    };

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_reg2, rand_imm0, rand_imm1;
    for(int i=0; i<NUM_FUZZ_TESTS; i++){

        rand_reg0 = rand() % 0xE;
        while((rand_reg1 = rand() % 0xE) == rand_reg0);
        while((rand_reg2 = rand() % 0xE) == rand_reg0 || rand_reg2 == rand_reg1);
        rand_imm0 = rand() % 0xFF;
        while((rand_imm1 = rand() % 0xFF) == rand_imm0);

        // std::cout << "Test #: " << std::setw(3) << std::setfill('0') << i <<    "; Reg0: 0x" << AS_HEX(1, rand_reg0) << 
        //                                                                         "; Reg1: 0x" << AS_HEX(1, rand_reg1) << 
        //                                                                         "; Reg2: 0x" << AS_HEX(1, rand_reg2) <<
        //                                                                         "; Imm0: 0x" << AS_HEX(1, rand_imm0) <<
        //                                                                         "; Imm1: 0x" << AS_HEX(1, rand_imm1) << std::endl;

        JMP_test_prog[0x06] = 0x60 | rand_reg0; // [0x206] LD V(rand_reg0) = rand_imm0
        JMP_test_prog[0x07] = rand_imm0;        // [0x207] LD V(rand_reg0) = rand_imm0
        JMP_test_prog[0x08] = 0x60 | rand_reg1; // [0x208] LD V(rand_reg1) = rand_imm0
        JMP_test_prog[0x09] = rand_imm1;        // [0x209] LD V(rand_reg1) = rand_imm0
        JMP_test_prog[0x0A] = 0x60 | rand_reg2; // [0x20A] LD V(rand_reg2) = rand_imm0
        JMP_test_prog[0x0B] = rand_imm0;        // [0x20B] LD V(rand_reg2) = rand_imm0
        JMP_test_prog[0x0C] = 0x30 | rand_reg2; // [0x20C] SE V(rand_reg2) == rand_imm1
        JMP_test_prog[0x0D] = rand_imm1;        // [0x20D] SE V(rand_reg0) == rand_imm1
        JMP_test_prog[0x0E] = 0x30 | rand_reg0; // [0x210] SE V(rand_reg0) == rand_imm0
        JMP_test_prog[0x0F] = rand_imm0;        // [0x211] SE V(rand_reg0) == rand_imm0
        JMP_test_prog[0x12] = 0x40 | rand_reg0; // [0x212] SNE V(rand_reg0) != rand_imm0
        JMP_test_prog[0x13] = rand_imm0;        // [0x213] SNE V(rand_reg0) != rand_imm0
        JMP_test_prog[0x14] = 0x40 | rand_reg1; // [0x214] SNE V(rand_reg1) != rand_imm0
        JMP_test_prog[0x15] = rand_imm0;        // [0x215] SNE V(rand_reg1) != rand_imm0
        JMP_test_prog[0x18] = 0x90 | rand_reg0; // [0x218] SNE V(rand_reg0) != V(rand_reg2)
        JMP_test_prog[0x19] = rand_reg2 << 4;   // [0x219] SNE V(rand_reg0) != V(rand_reg2)
        JMP_test_prog[0x1A] = 0x90 | rand_reg2; // [0x21A] SNE V(rand_reg2) != V(rand_reg1)
        JMP_test_prog[0x1B] = rand_reg1 << 4;   // [0x21B] SNE V(rand_reg2) != V(rand_reg1)
        JMP_test_prog[0x1E] = 0x50 | rand_reg0; // [0x21E] SE V(rand_reg0) != V(rand_reg1)
        JMP_test_prog[0x1F] = rand_reg1 << 4;   // [0x21F] SE V(rand_reg0) != V(rand_reg1)
        JMP_test_prog[0x20] = 0x50 | rand_reg2; // [0x220] SE V(rand_reg2) != V(rand_reg0)
        JMP_test_prog[0x21] = rand_reg0 << 4;   // [0x221] SE V(rand_reg2) != V(rand_reg0)



        membuff test_prog_stream((char*) JMP_test_prog, (char*) (JMP_test_prog + sizeof(JMP_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        for(int j=0; j<13; j++){
            RUN_TICK();
        }

    }
}

BOOST_AUTO_TEST_CASE(chip8_test_BITWISE){

    uint8_t BITWISE_test_prog[0x1C]; 

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xF;
        while((rand_reg1 = rand() % 0xF) == rand_reg0);
        (rand_imm0 = 0x01 | (rand() & 0x7F));
        (rand_imm1 = 0x80 | (rand() & 0xFE));

        BITWISE_test_prog[0x00] = 0x60 | rand_reg0;       // [0x200] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x01] = rand_imm0;              // [0x201] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x02] = 0x60 | rand_reg1;       // [0x202] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x03] = rand_imm1;              // [0x203] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x04] = 0x80 | rand_reg1;       // [0x204] OR V(rand_reg1) |= V(rand_reg0)
        BITWISE_test_prog[0x05] = (rand_reg0 << 4) | 0x1; // [0x205] OR V(rand_reg1) |= V(rand_reg0)
        BITWISE_test_prog[0x06] = 0x60 | rand_reg1;       // [0x206] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x07] = rand_imm1;              // [0x207] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x08] = 0x80 | rand_reg0;       // [0x208] AND V(rand_reg0) &= V(rand_reg1)
        BITWISE_test_prog[0x09] = (rand_reg1 << 4) | 0x2; // [0x209] AND V(rand_reg0) &= V(rand_reg1)
        BITWISE_test_prog[0x0A] = 0x60 | rand_reg0;       // [0x20A] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x0B] = rand_imm0;              // [0x20B] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x0C] = 0x80 | rand_reg1;       // [0x20C] XOR V(rand_reg1) ^= V(rand_reg0)
        BITWISE_test_prog[0x0D] = (rand_reg0 << 4) | 0x3; // [0x20D] XOR V(rand_reg1) ^= V(rand_reg0)
        BITWISE_test_prog[0x0E] = 0x60 | rand_reg1;       // [0x20E] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x0F] = rand_imm1;              // [0x20F] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x10] = 0x80 | rand_reg0;       // [0x210] SHR V(rand_reg0) >>= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x11] = 0x6;                    // [0x211] SHR V(rand_reg0) >>= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x12] = 0x60 | rand_reg0;       // [0x212] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x13] = rand_imm0;              // [0x213] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x14] = 0x80 | rand_reg1;       // [0x214] SHR V(rand_reg1) >>= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x15] = 0x6;                    // [0x215] SHR V(rand_reg1) >>= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x16] = 0x60 | rand_reg1;       // [0x216] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x17] = rand_imm1;              // [0x217] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x18] = 0x80 | rand_reg0;       // [0x218] SHL V(rand_reg0) <<= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x19] = 0xE;                    // [0x219] SHL V(rand_reg0) <<= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x1A] = 0x80 | rand_reg1;       // [0x21A] SHL V(rand_reg1) <<= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x1B] = 0xE;                    // [0x21b] SHL V(rand_reg1) <<= 1; **SHIFT 1 OUT**





        membuff test_prog_stream((char*) BITWISE_test_prog, (char*) (BITWISE_test_prog + sizeof(BITWISE_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        RUN_TICK();
        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 | rand_imm0) & 0xFF), "OR test case failed, incorrect bitwise OR result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> | V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 | rand_imm0) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 & rand_imm1) & 0xFF), "AND test case failed, incorrect bitwise AND result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> & V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 & rand_imm1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 ^ rand_imm0) & 0xFF), "XOR test case failed, incorrect bitwise XOR result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> ^ V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 ^ rand_imm0) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 >> 1) & 0xFF), "SHR test case failed, incorrect logical shift right result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 >> 1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x01, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 >> 1) & 0xFF), "SHR test case failed, incorrect logical shift right result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 >> 1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x00, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 << 1) & 0xFF), "SHR test case failed, incorrect logical shift left result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> << <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 >> 1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x00, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");

        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 << 1) & 0xFF), "SHR test case failed, incorrect logical shift left result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> << <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 >> 1) & 0xFF) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xF] == 0x01, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xF)) << ">");
    
    }
}
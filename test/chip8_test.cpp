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
        test_load[i] = (char) (rand() % 0xff);
    }

    membuff test_prog_stream((char*) test_load, (char*) (test_load + sizeof(test_load)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(0, test_prog_istream);

    TEST_ASSERT(memcmp(test_load, ch8.memory + PROG_START_OFFSET, sizeof(test_load)) == 0, "Init test fail; memory contents do not match loaded program.");
}

BOOST_AUTO_TEST_CASE (chip8_test_LD){

    const uint8_t LD_V_test_prog[] = {    0x60, 0xf0, // [0x200] LD V0 = 0xf0
                                                0x61, 0xf1, // [0x202] LD V1 = 0xf1
                                                0x62, 0xf2, // [0x204] LD V1 = 0xf2
                                                0x63, 0xf3, // [0x206] LD V1 = 0xf3
                                                0x64, 0xf4, // [0x208] LD V1 = 0xf4
                                                0x65, 0xf5, // [0x20a] LD V1 = 0xf5
                                                0x66, 0xf6, // [0x20c] LD V1 = 0xf6
                                                0x67, 0xf7, // [0x20e] LD V1 = 0xf7
                                                0x68, 0xf8, // [0x210] LD V1 = 0xf8
                                                0x69, 0xf9, // [0x212] LD V1 = 0xf9
                                                0x6a, 0xfa, // [0x214] LD V1 = 0xfa
                                                0x6b, 0xfb, // [0x216] LD V1 = 0xfb
                                                0x6c, 0xfc, // [0x218] LD V1 = 0xfc
                                                0x6d, 0xfd, // [0x220] LD V1 = 0xfd
                                                0x6e, 0xfe, // [0x222] LD V1 = 0xfe
                                                0x6f, 0xff, // [0x224] LD V1 = 0xff
                                                0x80, 0x80, // [0x226] LD V0 = V8
                                                0x81, 0x90, // [0x228] LD V1 = V9
                                                0x82, 0xa0, // [0x22a] LD V2 = VA
                                                0x83, 0xb0, // [0x22c] LD V3 = VB
                                                0x84, 0xc0, // [0x22e] LD V4 = VC
                                                0x85, 0xd0, // [0x230] LD V5 = VD
                                                0x86, 0xe0, // [0x232] LD V6 = VE
                                                0x87, 0xf0, // [0x234] LD V7 = VF
                                                0x00,}; 

    membuff test_prog_stream((char*) LD_V_test_prog, (char*) (LD_V_test_prog + sizeof(LD_V_test_prog)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(0, test_prog_istream);

    for(int count = 0; count < 0x10; count++){
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[count] == count + 0xf0, "Load test fail for register V(0x" << count << "); Expected: <0x" << AS_HEX(2, count + 0xf0) << ">; Actual: <0x" << AS_HEX(2, ch8.v_regs[count]) << ">");
    }

    for(int count = 0; count < 0x08; count++){
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[count] == ch8.v_regs[count + 8], "Load test fail for register V(0x" << count << "); Expected: <0x" << AS_HEX(2, ch8.v_regs[count + 8]) << ">; Actual: <0x" << AS_HEX(2, ch8.v_regs[count]) << ">");
    }

}

BOOST_AUTO_TEST_CASE(chip8_test_DT_ST){

    uint8_t DT_ST_test_prog[] = {   0x60, 0x00, // [0x200] LD V(rand_reg0) = rand_imm0
                                    0x60, 0x00, // [0x202] LD V(rand_reg1) = rand_imm1
                                    0xf0, 0x15, // [0x204] LD DT = V(rand_reg0)
                                    0xf0, 0x18, // [0x206] LD ST = V(rand_reg1)
                                    0xf0, 0x07, // [0x208] LD V(rand_reg1) = DT
                                    0x30, 0x00, // [0x20a] SE V(rand_reg1) == 0x00
                                    0x12, 0x08, // [0x20c] JMP 0x208
                                    0x00, 0x00, // [0x20e] NOP
    };

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xf;
        while((rand_reg1 = rand() % 0xf) == rand_reg0);
        rand_imm1 = rand() & 0xff;
        while((rand_imm0 = rand() & 0xff) < rand_imm1);

        DT_ST_test_prog[0x00] = 0x60 | rand_reg0;   // [0x200] LD V(rand_reg0) = rand_imm0
        DT_ST_test_prog[0x01] = rand_imm0;          // [0x201] LD V(rand_reg0) = rand_imm0
        DT_ST_test_prog[0x02] = 0x60 | rand_reg1;   // [0x202] LD V(rand_reg1) = rand_imm1
        DT_ST_test_prog[0x03] = rand_imm1;          // [0x203] LD V(rand_reg1) = rand_imm1
        DT_ST_test_prog[0x04] = 0xf0 | rand_reg0;   // [0x204] LD DT = V(rand_reg0)
        DT_ST_test_prog[0x06] = 0xf0 | rand_reg1;   // [0x206] LD ST = V(rand_reg1)
        DT_ST_test_prog[0x08] = 0xf0 | rand_reg0;   // [0x208] LD V(rand_reg0) = DT
        DT_ST_test_prog[0x0a] = 0x30 | rand_reg0;   // [0x20a] SE V(rand_reg0) == 0x00

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
            TEST_ASSERT(ch8.v_regs[rand_reg0] == ch8.dt_reg, "Load test case failed " << static_cast<uint>(rand_reg0) << "; " << static_cast<uint>(rand_reg1) << "; " << static_cast<uint>(rand_imm0) << "; " << static_cast<uint>(rand_imm1) << " V(0x" << AS_HEX(1, rand_reg0) << ") = DT<0x" << AS_HEX(2, ch8.dt_reg) << ">; Expected V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, ch8.dt_reg) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, ch8.v_regs[rand_reg0]) << ">");
            
            RUN_TICK();
            ticks += 2;
            if(ticks < 10 * rand_imm0){
                RUN_TICK();
                ticks++;

                TEST_ASSERT(ch8.dt_reg == ((rand_imm0 - ticks / 10 > 0)? rand_imm0 - ticks / 10 : 0), "DT counter test case failed; Expected DT<0x" << AS_HEX(2, ((rand_imm0 - ticks / 10) > 0)? rand_imm0 - ticks / 10 : 0) << ">; Actual DT<0x" << AS_HEX(2, ch8.dt_reg) << ">");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE (chip8_test_LD_I){
    const uint8_t LD_i_test_prog[] = {    0x60, 0x00, // [0x200] LD V0 = 0x00
                                                0xf0, 0x29, // [0x202] LD I = Addr of sprite of the value of V0
                                                0x61, 0x01, // [0x204] LD V1 = 0x01
                                                0xf1, 0x29, // [0x206] LD I = Addr of sprite of the value of V1
                                                0x62, 0x02, // [0x208] LD V2 = 0x02
                                                0xf2, 0x29, // [0x20a] LD I = Addr of sprite of the value of V2
                                                0x63, 0x03, // [0x20c] LD V3 = 0x03
                                                0xf3, 0x29, // [0x20e] LD I = Addr of sprite of the value of V3
                                                0x64, 0x04, // [0x210] LD V4 = 0x04
                                                0xf4, 0x29, // [0x212] LD I = Addr of sprite of the value of V4
                                                0x65, 0x05, // [0x214] LD V5 = 0x05
                                                0xf5, 0x29, // [0x216] LD I = Addr of sprite of the value of V5
                                                0x66, 0x06, // [0x218] LD V6 = 0x06
                                                0xf6, 0x29, // [0x21a] LD I = Addr of sprite of the value of V6
                                                0x67, 0x07, // [0x21b] LD V7 = 0x07
                                                0xf7, 0x29, // [0x21e] LD I = Addr of sprite of the value of V7
                                                0x68, 0x08, // [0x220] LD V8 = 0x08
                                                0xf8, 0x29, // [0x222] LD I = Addr of sprite of the value of V8
                                                0x69, 0x09, // [0x224] LD V9 = 0x09
                                                0xf9, 0x29, // [0x226] LD I = Addr of sprite of the value of V9
                                                0x6a, 0x0a, // [0x228] LD VA = 0x0a
                                                0xfa, 0x29, // [0x22a] LD I = Addr of sprite of the value of VA
                                                0x6b, 0x0b, // [0x22c] LD VB = 0x0b
                                                0xfb, 0x29, // [0x22e] LD I = Addr of sprite of the value of VB
                                                0x6c, 0x0c, // [0x230] LD VC = 0x0c
                                                0xfc, 0x29, // [0x232] LD I = Addr of sprite of the value of VC
                                                0x6d, 0x0d, // [0x234] LD VD = 0x0d
                                                0xfd, 0x29, // [0x236] LD I = Addr of sprite of the value of VD
                                                0x6e, 0x0e, // [0x238] LD VE = 0x0e
                                                0xfe, 0x29, // [0x23a] LD I = Addr of sprite of the value of VE
                                                0x6f, 0x0f, // [0x23b] LD VF = 0x0f
                                                0xff, 0x29, // [0x23c] LD I = Addr of sprite of the value of VF
                                                0xaf, 0xf0, // [0x23e] LD I = 0xff0
                                                0x60, 0x9d, // [0x240] LD V0 = 0x9d (157)
                                                0xf0, 0x33, // [0x242] LD BCD representation of V0 into memory[I], memory[I+1], memory[I+2]
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
    TEST_ASSERT(ch8.i_reg == 0xff0, "Load test case failed for register I; Expected: <0x" << AS_HEX(4, 0xff0) << ">; Actual: <0x" << AS_HEX(4, ch8.i_reg) << ">");

    // LD BCD repr of V0 into memory[I, I+1, I+2]
    RUN_TICK();
    RUN_TICK();
    const char expected_bcd[] = {1, 5, 7};
    TEST_ASSERT(memcmp(expected_bcd, ch8.memory + 0x0ff0, 3) == 0, "Load teast failed for BCD repr load of V(0) = <0x9d> (157); Expected: {1, 5, 7}; Actual: {" << uint(ch8.memory[ch8.i_reg]) << ", " << uint(ch8.memory[ch8.i_reg + 1]) << ", " << uint(ch8.memory[ch8.i_reg + 2]) << "}");
}

BOOST_AUTO_TEST_CASE (chip8_test_ADD){

    uint8_t ADD_test_prog[12];

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xf;
        while((rand_reg1 = rand() % 0xf) == rand_reg0);
        rand_imm0 = rand() % 0x7f;
        rand_imm1 = 0x80 + rand() % 0x7f;

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
        ADD_test_prog[10] = 0x80 | rand_reg0;       // [0x20a] ADD V(rand_reg0) += V(rand_reg0) **NO OVERFLOW**
        ADD_test_prog[11] = (rand_reg0 << 4) | 0x04;// [0x20b] ADD V(rand_reg0) += V(rand_reg0) **NO OVERFLOW**

        membuff test_prog_stream((char*) ADD_test_prog, (char*) (ADD_test_prog + sizeof(ADD_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(0, test_prog_istream);

        RUN_TICK();
        RUN_TICK();

        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 + rand_imm0) & 0xff), "ADD imm test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0)<< ")<0x" << AS_HEX(2, (rand_imm0 + rand_imm0) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0)<< ")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 + rand_imm1) & 0xff), "ADD reg test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> + <0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1)<< ")<0x" << AS_HEX(2, (rand_imm1 + rand_imm1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1)<< ")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x01, "ADD reg test case failed, incorrect carry flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> + <0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 + rand_imm0) & 0xff), "ADD reg test case failed, incorrect add result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: <0x" << AS_HEX(2, (rand_imm0 + rand_imm0) & 0xff) << ">; Actual <0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x00, "ADD reg test case failed, incorrect carry flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> + <0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

    }
}

BOOST_AUTO_TEST_CASE (chip8_test_SUB){

    uint8_t SUB_test_prog[18]; 

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xf;
        while((rand_reg1 = rand() % 0xf) == rand_reg0);
        rand_imm0 = rand() % 0x7f;
        rand_imm1 = 0x80 + rand() % 0x7f;

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
        SUB_test_prog[0x0a] = 0x60 | rand_reg0;       // [0x20a] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x0b] = rand_imm0;              // [0x20b] LD V(rand_reg0) = rand_imm0
        SUB_test_prog[0x0c] = 0x80 | rand_reg1;       // [0x20c] SUBN V(rand_reg1) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x0d] = (rand_reg0 << 4) | 0x7; // [0x20d] SUBN V(rand_reg1) = V(rand_reg0) - V(rand_reg1) **BORROW**
        SUB_test_prog[0x0e] = 0x60 | rand_reg1;       // [0x20e] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x0f] = rand_imm1;              // [0x20f] LD V(rand_reg1) = rand_imm1
        SUB_test_prog[0x10] = 0x80 | rand_reg0;       // [0x210] SUBN V(rand_reg0) = V(rand_reg1) - V(rand_reg0) **NO BORROW**
        SUB_test_prog[0x11] = (rand_reg1 << 4) | 0x7; // [0x211] SUBN V(rand_reg0) = V(rand_reg1) - V(rand_reg0) **NO BORROW**


        membuff test_prog_stream((char*) SUB_test_prog, (char*) (SUB_test_prog + sizeof(SUB_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        RUN_TICK();
        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 - rand_imm0) & 0xff), "SUB test case failed, incorrect SUB result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 - rand_imm0) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x01, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 - rand_imm1) & 0xff), "SUB test case failed, incorrect SUB result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 - rand_imm1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x00, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");


        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm0 - rand_imm1) & 0xff), "SUB test case failed, incorrect SUBN result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm0 - rand_imm1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x00, "SUBN test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> - V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm1 - rand_imm0) & 0xff), "SUB test case failed, incorrect SUBN result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm1 - rand_imm0) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x01, "SUB test case failed, incorrect borrow flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> - V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

    }

}

BOOST_AUTO_TEST_CASE(chip8_test_JMP){

    // test succeeds if no nop errors occur
    uint8_t JMP_test_prog[] = { 0x22, 0x2c, // [0x200] CALL 0x22c
                                0x12, 0x08, // [0x202] JMP 0x208
                                0x00, 0x00, // [0x204] NOP
                                0x00, 0x00, // [0x206] NOP
                                0x60, 0x00, // [0x208] LD V(rand_reg0) = rand_imm0
                                0x60, 0x00, // [0x20a] LD V(rand_reg1) = rand_imm1
                                0x60, 0x00, // [0x20c] LD V(rand_reg2) = rand_imm0
                                0x30, 0x00, // [0x20e] SE V(rand_reg2) == rand_imm1     **NO SKIP**
                                0x30, 0x00, // [0x210] SE V(rand_reg0) == rand_imm0     **SKIP**
                                0x00, 0x00, // [0x212] NOP
                                0x40, 0x00, // [0x214] SNE V(rand_reg0) != rand_imm0     **NO SKIP**
                                0x40, 0x00, // [0x216] SNE V(rand_reg1) != rand_imm0    **SKIP**
                                0x00, 0x00, // [0x218] NOP
                                0x90, 0x00, // [0x21a] SNE V(rand_reg0) != V(rand_reg2)   **NO SKIP**
                                0x90, 0x00, // [0x21c] SNE V(rand_reg2) != V(rand_reg1) ** SKIP**
                                0x00, 0x00, // [0x21e] NOP
                                0x50, 0x00, // [0x220] SE V(rand_reg0) == V(rand_reg1)  **NO SKIP**
                                0x50, 0x00, // [0x222] SE V(rand_reg2) == V(rand_reg0)  **SKIP** 
                                0x00, 0x00, // [0x224] NOP
                                0x60, 0x00, // [0x226] LD V(0) = 0 
                                0x00, 0x00, // [0x228] NOP
                                0x00, 0x00, // [0x22a] NOP
                                0x00, 0xee, // [0x22c] RET
    };

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_reg2, rand_imm0, rand_imm1;
    for(int i=0; i<NUM_FUZZ_TESTS; i++){

        rand_reg0 = rand() % 0xe;
        while((rand_reg1 = rand() % 0xe) == rand_reg0);
        while((rand_reg2 = rand() % 0xe) == rand_reg0 || rand_reg2 == rand_reg1);
        rand_imm0 = rand() % 0xff;
        while((rand_imm1 = rand() % 0xff) == rand_imm0);

        // std::cout << "Test #: " << std::setw(3) << std::setfill('0') << i <<    "; Reg0: 0x" << AS_HEX(1, rand_reg0) << 
        //                                                                         "; Reg1: 0x" << AS_HEX(1, rand_reg1) << 
        //                                                                         "; Reg2: 0x" << AS_HEX(1, rand_reg2) <<
        //                                                                         "; Imm0: 0x" << AS_HEX(1, rand_imm0) <<
        //                                                                         "; Imm1: 0x" << AS_HEX(1, rand_imm1) << std::endl;

        JMP_test_prog[0x08] = 0x60 | rand_reg0; // [0x208] LD V(rand_reg0) = rand_imm0
        JMP_test_prog[0x09] = rand_imm0;        // [0x209] LD V(rand_reg0) = rand_imm0
        JMP_test_prog[0x0a] = 0x60 | rand_reg1; // [0x20a] LD V(rand_reg1) = rand_imm0
        JMP_test_prog[0x0b] = rand_imm1;        // [0x20b] LD V(rand_reg1) = rand_imm0
        JMP_test_prog[0x0c] = 0x60 | rand_reg2; // [0x20c] LD V(rand_reg2) = rand_imm0
        JMP_test_prog[0x0d] = rand_imm0;        // [0x20d] LD V(rand_reg2) = rand_imm0
        JMP_test_prog[0x0e] = 0x30 | rand_reg2; // [0x20e] SE V(rand_reg2) == rand_imm1
        JMP_test_prog[0x0f] = rand_imm1;        // [0x20f] SE V(rand_reg0) == rand_imm1
        JMP_test_prog[0x10] = 0x30 | rand_reg0; // [0x210] SE V(rand_reg0) == rand_imm0
        JMP_test_prog[0x11] = rand_imm0;        // [0x211] SE V(rand_reg0) == rand_imm0
        JMP_test_prog[0x14] = 0x40 | rand_reg0; // [0x214] SNE V(rand_reg0) != rand_imm0
        JMP_test_prog[0x15] = rand_imm0;        // [0x215] SNE V(rand_reg0) != rand_imm0
        JMP_test_prog[0x16] = 0x40 | rand_reg1; // [0x216] SNE V(rand_reg1) != rand_imm0
        JMP_test_prog[0x17] = rand_imm0;        // [0x217] SNE V(rand_reg1) != rand_imm0
        JMP_test_prog[0x1a] = 0x90 | rand_reg0; // [0x21a] SNE V(rand_reg0) != V(rand_reg2)
        JMP_test_prog[0x1b] = rand_reg2 << 4;   // [0x21b] SNE V(rand_reg0) != V(rand_reg2)
        JMP_test_prog[0x1c] = 0x90 | rand_reg2; // [0x21c] SNE V(rand_reg2) != V(rand_reg1)
        JMP_test_prog[0x1d] = rand_reg1 << 4;   // [0x21d] SNE V(rand_reg2) != V(rand_reg1)
        JMP_test_prog[0x20] = 0x50 | rand_reg0; // [0x220] SE V(rand_reg0) == V(rand_reg1)
        JMP_test_prog[0x21] = rand_reg1 << 4;   // [0x221] SE V(rand_reg0) == V(rand_reg1)
        JMP_test_prog[0x22] = 0x50 | rand_reg2; // [0x222] SE V(rand_reg2) == V(rand_reg0)
        JMP_test_prog[0x23] = rand_reg0 << 4;   // [0x223] SE V(rand_reg2) == V(rand_reg0)



        membuff test_prog_stream((char*) JMP_test_prog, (char*) (JMP_test_prog + sizeof(JMP_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        for(int j=0; j<15; j++){
            RUN_TICK();
        }

    }
}

BOOST_AUTO_TEST_CASE(chip8_test_CALL){
    uint8_t CALL_test_prog[] = {0x22, 0x18, // [0x200] CALL 0x218
                                0x60, 0x00, // [0x202] LD V(0) = 0
                                0x00, 0x00, // [0x204] NOP
                                0x22, 0x1c, // [0x206] CALL 0x21c
                                0x00, 0xee, // [0x208] RET
                                0x22, 0x20, // [0x20a] CALL 0x220
                                0x00, 0xee, // [0x20c] RET
                                0x22, 0x2c, // [0x20e] CALL 0x22c
                                0x00, 0xee, // [0x210] RET
                                0x00, 0x00, // [0x212] NOP
                                0x22, 0x24, // [0x214] CALL 0x224
                                0x00, 0xee, // [0x216] RET
                                0x22, 0x06, // [0x218] CALL 0x206
                                0x00, 0xee, // [0x21a] RET
                                0x22, 0x28, // [0x21c] CALL 0x228
                                0x00, 0xee, // [0x21e] RET
                                0x22, 0x14, // [0x220] CALL 0x214
                                0x00, 0xee, // [0x222] RET
                                0x22, 0x0e, // [0x224] CALL 0x20e
                                0x00, 0xee, // [0x226] RET
                                0x22, 0x0a, // [0x228] CALL 0x20a
                                0x00, 0xee, // [0x22a] RET
                                0x22, 0x3c, // [0x22c] CALL 0x23c
                                0x00, 0xee, // [0x22e] RET
                                0x22, 0x40, // [0x230] CALL 0x240
                                0x00, 0xee, // [0x232] RET
                                0x22, 0x42, // [0x234] CALL 0x242
                                0x00, 0xee, // [0x236] RET
                                0x22, 0x34, // [0x238] CALL 0x234
                                0x00, 0xee, // [0x23a] RET
                                0x22, 0x30, // [0x23c] CALL 0x230
                                0x00, 0xee, // [0x23e] RET
                                0x22, 0x38, // [0x240] CALL 0x238
                                0x00, 0xee, // [0x242] RET
    };

    membuff test_prog_stream((char*) CALL_test_prog, (char*) (CALL_test_prog + sizeof(CALL_test_prog)));
    std::istream test_prog_istream(&test_prog_stream);

    chip8 ch8(time(NULL), test_prog_istream);

    for(int j=0; j<33; j++){
        RUN_TICK();
    }
}

BOOST_AUTO_TEST_CASE(chip8_test_BITWISE){

    uint8_t BITWISE_test_prog[0x1c]; 

    SEED_RAND();

    uint8_t rand_reg0, rand_reg1, rand_imm0, rand_imm1;
    for(int i = 0; i< NUM_FUZZ_TESTS; i++){
        rand_reg0 = rand() % 0xf;
        while((rand_reg1 = rand() % 0xf) == rand_reg0);
        (rand_imm0 = 0x01 | (rand() & 0x7f));
        (rand_imm1 = 0x80 | (rand() & 0xfe));

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
        BITWISE_test_prog[0x0a] = 0x60 | rand_reg0;       // [0x20a] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x0b] = rand_imm0;              // [0x20b] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x0c] = 0x80 | rand_reg1;       // [0x20c] XOR V(rand_reg1) ^= V(rand_reg0)
        BITWISE_test_prog[0x0d] = (rand_reg0 << 4) | 0x3; // [0x20d] XOR V(rand_reg1) ^= V(rand_reg0)
        BITWISE_test_prog[0x0e] = 0x60 | rand_reg1;       // [0x20e] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x0f] = rand_imm1;              // [0x20f] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x10] = 0x80 | rand_reg0;       // [0x210] SHR V(rand_reg0) >>= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x11] = 0x6;                    // [0x211] SHR V(rand_reg0) >>= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x12] = 0x60 | rand_reg0;       // [0x212] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x13] = rand_imm0;              // [0x213] LD V(rand_reg0) = rand_imm0
        BITWISE_test_prog[0x14] = 0x80 | rand_reg1;       // [0x214] SHR V(rand_reg1) >>= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x15] = 0x6;                    // [0x215] SHR V(rand_reg1) >>= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x16] = 0x60 | rand_reg1;       // [0x216] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x17] = rand_imm1;              // [0x217] LD V(rand_reg1) = rand_imm1
        BITWISE_test_prog[0x18] = 0x80 | rand_reg0;       // [0x218] SHL V(rand_reg0) <<= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x19] = 0xe;                    // [0x219] SHL V(rand_reg0) <<= 1; **SHIFT 0 OUT**
        BITWISE_test_prog[0x1a] = 0x80 | rand_reg1;       // [0x21a] SHL V(rand_reg1) <<= 1; **SHIFT 1 OUT**
        BITWISE_test_prog[0x1b] = 0xe;                    // [0x21b] SHL V(rand_reg1) <<= 1; **SHIFT 1 OUT**





        membuff test_prog_stream((char*) BITWISE_test_prog, (char*) (BITWISE_test_prog + sizeof(BITWISE_test_prog)));
        std::istream test_prog_istream(&test_prog_stream);

        chip8 ch8(time(NULL), test_prog_istream);

        RUN_TICK();
        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 | rand_imm0) & 0xff), "OR test case failed, incorrect bitwise OR result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> | V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 | rand_imm0) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 & rand_imm1) & 0xff), "AND test case failed, incorrect bitwise AND result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> & V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << ">; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 & rand_imm1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 ^ rand_imm0) & 0xff), "XOR test case failed, incorrect bitwise XOR result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> ^ V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << ">; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 ^ rand_imm0) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 >> 1) & 0xff), "SHR test case failed, incorrect logical shift right result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 >> 1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x01, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 >> 1) & 0xff), "SHR test case failed, incorrect logical shift right result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 >> 1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x00, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg0] == ((rand_imm0 << 1) & 0xff), "SHR test case failed, incorrect logical shift left result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> << <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, (rand_imm0 >> 1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg0) <<")<0x" << AS_HEX(2, ch8.V(rand_reg0)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x00, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg0) << ") = V(0x" << AS_HEX(1, rand_reg0) << ")<0x" << AS_HEX(2, rand_imm0) << "> >> <0x1>; Expected: V(F)<0x00>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");

        RUN_TICK();
        TEST_ASSERT(ch8.v_regs[rand_reg1] == ((rand_imm1 << 1) & 0xff), "SHR test case failed, incorrect logical shift left result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> << <0x1>; Expected: V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, (rand_imm1 >> 1) & 0xff) << ">; Actual V(0x" << AS_HEX(1, rand_reg1) <<")<0x" << AS_HEX(2, ch8.V(rand_reg1)) << ">");
        TEST_ASSERT(ch8.v_regs[0xf] == 0x01, "SHR test case failed, incorrect shift out flag result; V(0x" << AS_HEX(1, rand_reg1) << ") = V(0x" << AS_HEX(1, rand_reg1) << ")<0x" << AS_HEX(2, rand_imm1) << "> >> <0x1>; Expected: V(F)<0x01>; Actual: V(F)<0x" << AS_HEX(2, ch8.V(0xf)) << ">");
    
    }
}
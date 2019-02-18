#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Chip8
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/cstdint.hpp>

#include <iostream>
#include <iomanip>
#include <random>
#include <ctime>
#include <random>
#include <chrono>
#include <climits>
#include <iterator>

#include "../src/Chip8.hpp"

#define NUM_DATA_TESTS 1024

#define AS_HEX(width, value) std::hex << std::setw(width) << std::setfill('0') << static_cast<uint>(value)

// Create test subclass to expose protected members
class Chip8Test : public Chip8::Chip8{
public:
    Chip8Test(std::mt19937::result_type t_seed) : Chip8(t_seed) {}
    Chip8Test(std::mt19937::result_type t_seed, const std::string& t_romPath) : Chip8(t_seed, t_romPath){}
    Chip8Test(std::mt19937::result_type t_seed, std::istream& t_romStream) : Chip8(t_seed, t_romStream){}

    // Expose protected member variables
    using Chip8::m_seed;
    using Chip8::m_memory;
    using Chip8::m_vRegs;
    using Chip8::m_stack;
    using Chip8::m_disp;
    using Chip8::m_iReg;
    using Chip8::m_dtReg;
    using Chip8::m_stReg;
    using Chip8::m_stackPointer;
    using Chip8::m_programCounter;
    using Chip8::m_tCounter;
    using Chip8::m_keystates;

    // Expose protected member functions
    using Chip8::CLS;
    using Chip8::RET;
    using Chip8::JMP;
    using Chip8::CALL;
    using Chip8::SE_IMM;
    using Chip8::SNE_IMM;
    using Chip8::SE_REG;
    using Chip8::LD_IMM;
    using Chip8::ADD_IMM;
    using Chip8::LD_REG;
    using Chip8::OR;
    using Chip8::AND;
    using Chip8::XOR;
    using Chip8::ADD_REG;
    using Chip8::SUB_REG;
    using Chip8::SHR;
    using Chip8::SUBN;
    using Chip8::SHL;
    using Chip8::SNE_REG;
    using Chip8::LD_I;
    using Chip8::JMP_REG;
    using Chip8::RND;
    using Chip8::DRW;
    using Chip8::SKP;
    using Chip8::SKNP;
    using Chip8::LD_VX_DT;
    using Chip8::LD_KP;
    using Chip8::LD_DT_VX;
    using Chip8::LD_ST;
    using Chip8::ADD_I;
    using Chip8::LD_SPRT;
    using Chip8::LD_BCD;
    using Chip8::LD_MEM;
    using Chip8::LD_REGS;

    ~Chip8Test() = default;

};

struct TestBuf : std::streambuf{
    TestBuf(char* beg, char* end){
        this->setg(beg, beg, end);
    }
};

struct ArgsFixture{
    int argc;
    char** argv;

    ArgsFixture() : argc(boost::unit_test::framework::master_test_suite().argc),
                    argv(boost::unit_test::framework::master_test_suite().argv){}
};

namespace BoostData = boost::unit_test::data;
namespace arg = std::placeholders;

BOOST_GLOBAL_FIXTURE(ArgsFixture);

struct RandGeneratorFixture{

    std::mt19937::result_type seed;
    std::uniform_int_distribution<> byteDistribution;
    std::uniform_int_distribution<> nibbleDistribution;
    std::mt19937 generator;
    
    RandGeneratorFixture(){
        bool noSeed = false;
        for(int i = 0; i < boost::unit_test::framework::master_test_suite().argc; ++i){
            noSeed = noSeed || strcmp(boost::unit_test::framework::master_test_suite().argv[i], "--no_seed");
        }
        if(!noSeed){
            RandGeneratorFixture(std::chrono::system_clock::now().time_since_epoch().count());
        }
        else{
            RandGeneratorFixture(std::mt19937::default_seed);
        }
    }

    RandGeneratorFixture(std::mt19937::result_type t_seed) : seed(t_seed), byteDistribution(0, 0xff), nibbleDistribution(0, 0xf), generator(t_seed){
    }

    void seedGenerator(){
        generator = std::mt19937(seed);
    }

    uint8_t randNibble(){
        return nibbleDistribution(generator);
    }

    void randNibble(uint8_t& res){
        res = static_cast<uint8_t>(nibbleDistribution(generator));
    }

    uint8_t randByte(){
        return byteDistribution(generator);
    }

    void randByte(uint8_t& res){
        res = static_cast<uint8_t>(byteDistribution(generator));
    }

    template<typename TRes, typename UDistType>
    void rand(const UDistType& t_dist, TRes& res){
        res = static_cast<TRes>(t_dist(generator));
    }

    template<typename TRes, typename UDistType>
    TRes rand(const UDistType& t_dist){
        return static_cast<TRes>(t_dist(generator));
    }
};


BOOST_AUTO_TEST_CASE(Chip8Test_init){

    uint8_t testLoad[CHIP8_MAIN_MEM_SIZE - CHIP8_PROG_START_OFFSET];

    for(uint i = 0; i < sizeof(testLoad); ++i){
        testLoad[i] = (char) (rand() % 0xff);
    }

    TestBuf testProgStream((char*) testLoad, (char*) (testLoad + sizeof(testLoad)));
    std::istream testProgiStream(&testProgStream);

    Chip8Test chip8TestInst(static_cast<uint>(0), testProgiStream);

    BOOST_REQUIRE_MESSAGE(std::equal(std::begin(testLoad), std::end(testLoad), chip8TestInst.m_memory.begin() + CHIP8_PROG_START_OFFSET), "Test Failed, memory does contain loaded program.");
}

BOOST_FIXTURE_TEST_CASE(Chip8Test_CLS, RandGeneratorFixture){

    constexpr uint16_t dispSize = (CHIP8_DISP_X >> 3) * CHIP8_DISP_Y;

    Chip8Test chip8TestInst(std::mt19937::default_seed);

    std::array<uint8_t, dispSize> testLoad;
    std::for_each(testLoad.begin(), testLoad.end(), std::bind(static_cast<void(RandGeneratorFixture::*)(uint8_t&)>(&RandGeneratorFixture::randByte), this, arg::_1));

    std::array<uint8_t, dispSize> expected({0});

    std::copy(chip8TestInst.m_disp.begin(), chip8TestInst.m_disp.end(), testLoad.begin());

    chip8TestInst.CLS();

    BOOST_REQUIRE_MESSAGE(std::equal(chip8TestInst.m_disp.begin(), chip8TestInst.m_disp.end(), expected.begin()), "Error: CLS test fail, display not cleared.");
}

BOOST_DATA_TEST_CASE(Chip8Test_JMP, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, initAddr, callAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.JMP(2* callAddr);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == 2 * callAddr, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, 2 * callAddr) << "'; actual: '" << AS_HEX(3, chip8TestInst.m_programCounter));
}

BOOST_DATA_TEST_CASE(Chip8Test_CALL, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xf) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, initAddr, initStackPointer, callAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.CALL(2* callAddr);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_stackPointer == (initStackPointer)? initStackPointer - 1 : 0xf, "Test #" << testNumber << "failed, incorrect stack pointer result, expected '" << AS_HEX(1, (initStackPointer)? initStackPointer - 1 : 0xf) << "'; actual: '" << AS_HEX(1, chip8TestInst.m_stackPointer));
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == 2 * callAddr, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, 2 * callAddr) << "'; actual: '" << AS_HEX(3, chip8TestInst.m_programCounter));
}

BOOST_DATA_TEST_CASE(Chip8Test_RET, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xf) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, initAddr, initStackPointer, callAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.CALL(2* callAddr);
    chip8TestInst.RET();
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_stackPointer == (initStackPointer)? initStackPointer - 1 : 0xf, "Test #" << testNumber << "failed, incorrect stack pointer result, expected '" << AS_HEX(1, (initStackPointer)? initStackPointer - 1 : 0xf) << "'; actual: '" << AS_HEX(1, chip8TestInst.m_stackPointer));
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == 2 * initAddr, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, 2 * initAddr) << "'; actual: '" << AS_HEX(3, chip8TestInst.m_programCounter));
}

BOOST_DATA_TEST_CASE(Chip8Test_SE_IMM, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff) , testNumber, initAddr, registerIndex, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex, immediateValue0);
    chip8TestInst.SE_IMM(registerIndex, immediateValue0);
    uint expected = 2 * initAddr + 2;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for immediateValue0, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
    chip8TestInst.SE_IMM(registerIndex, immediateValue1);
    expected += (immediateValue1 == immediateValue0)? 2 : 0;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for immediateValue1, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_SNE_IMM, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff) , testNumber, initAddr, registerIndex, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex, immediateValue0);
    chip8TestInst.SNE_IMM(registerIndex, immediateValue0);
    uint expected = 2 * initAddr;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for immediateValue0, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
    chip8TestInst.SNE_IMM(registerIndex, immediateValue1);
    expected += (immediateValue1 != immediateValue0)? 2 : 0;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for immediateValue1, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_SE_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff) , testNumber, initAddr, registerIndex0, registerIndex1, registerIndex2, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue0);
    chip8TestInst.SE_REG(registerIndex0, registerIndex1);
    uint expected = 2 * initAddr + 2;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for value0, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
    chip8TestInst.LD_IMM(registerIndex2, immediateValue1);
    chip8TestInst.SE_REG(registerIndex0, registerIndex2);
    expected += (immediateValue0 == immediateValue1 || registerIndex0 == registerIndex2)? 2 : 0;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for value1, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_IMM, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);

    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == immediateValue, "Test #" << testNumber << " failed, incorrect load result, expected: '" << AS_HEX(2, immediateValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_ADD_IMM, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue0);
    chip8TestInst.ADD_IMM(registerIndex, immediateValue1);

    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == ((immediateValue0 + immediateValue1) & 0xff), "Test #" << testNumber << " failed, incorrect add result, expected: '" << AS_HEX(2, ((immediateValue0 + immediateValue1) & 0xff)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue);
    chip8TestInst.LD_REG(registerIndex1, registerIndex0);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex1] == immediateValue, "Test #" << testNumber << " failed, incorrect load result, expected: '" << AS_HEX(2, immediateValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex1]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_OR, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.OR(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == immediateValue1, "Test #" << testNumber << " failed, incorrect bitwise or result, expected: '" << AS_HEX(2, immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");

    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == (immediateValue0 | immediateValue1), "Test #" << testNumber << " failed, incorrect bitwise or result, expected: '" << AS_HEX(2, immediateValue0 | immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_AND, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.AND(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == immediateValue1, "Test #" << testNumber << " failed, incorrect bitwise and result, expected: '" << AS_HEX(2, immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");

    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == (immediateValue0 & immediateValue1), "Test #" << testNumber << " failed, incorrect bitwise and result, expected: '" << AS_HEX(2, immediateValue0 & immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_XOR, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.XOR(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == 0x00, "Test #" << testNumber << " failed, incorrect bitwise xor result, expected: '" << AS_HEX(2, immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");

    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == (immediateValue0 ^ immediateValue1), "Test #" << testNumber << " failed, incorrect bitwise xor result, expected: '" << AS_HEX(2, immediateValue0 ^ immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_ADD_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.ADD_REG(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == ((immediateValue1 + immediateValue1) & 0xff), "Test #" << testNumber << " failed, incorrect add result, expected: '" << AS_HEX(2, (immediateValue1 + immediateValue1) & 0xff) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(immediateValue1 > (UINT8_MAX - immediateValue1)), "Test #" << testNumber << " failed, incorrect overflow flag result, expected: '" << AS_HEX(2, immediateValue1 > (UINT8_MAX - immediateValue1)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == ((immediateValue0 + immediateValue1) & 0xff), "Test #" << testNumber << " failed, incorrect add result, expected: '" << AS_HEX(2, (immediateValue0 + immediateValue1) & 0xff) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(immediateValue0 > (UINT8_MAX - immediateValue1)), "Test #" << testNumber << " failed, incorrect overflow flag result, expected: '" << AS_HEX(2, immediateValue0 > (UINT8_MAX - immediateValue1)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_SUB_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.SUB_REG(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == 0x00, "Test #" << testNumber << " failed, incorrect sub result, expected: '" << AS_HEX(2, 0x00) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == 0x00, "Test #" << testNumber << " failed, incorrect borrow flag result, expected: '" << AS_HEX(2, 0x00) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == static_cast<uint8_t>(immediateValue0 - immediateValue1), "Test #" << testNumber << " failed, incorrect sub result, expected: '" << AS_HEX(2, static_cast<uint8_t>(immediateValue0 - immediateValue1)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(immediateValue0 > immediateValue1), "Test #" << testNumber << " failed, incorrect borrow flag result, expected: '" << AS_HEX(2, immediateValue0 > immediateValue1) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_SHR, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.SHR(registerIndex);

    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == (immediateValue >> 1), "Test #" << testNumber << " failed, incorrect shift right result, expected: '" << AS_HEX(2, (immediateValue >> 1)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(immediateValue & 0x01), "Test #" << testNumber << " failed, lsb flag result, expected: '" << AS_HEX(2, static_cast<uint8_t>(immediateValue & 0x01)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_SUBN, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff), testNumber, registerIndex0, registerIndex1, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue1);
    chip8TestInst.SUBN(registerIndex0, registerIndex1);
    if(registerIndex0 == registerIndex1){
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == 0x00, "Test #" << testNumber << " failed, incorrect subn result, expected: '" << AS_HEX(2, 0x00) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == 0x00, "Test #" << testNumber << " failed, incorrect subn borrow flag result, expected: '" << AS_HEX(2, 0x00) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
    else{
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex0] == static_cast<uint8_t>(immediateValue1 - immediateValue0), "Test #" << testNumber << " failed, incorrect subn result, expected: '" << AS_HEX(2, static_cast<uint8_t>(immediateValue1 - immediateValue0)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex0]) << "'");
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(immediateValue1 > immediateValue0), "Test #" << testNumber << " failed, incorrect subn borrow flag result, expected: '" << AS_HEX(2, immediateValue1 > immediateValue0) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_SHL, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.SHL(registerIndex);

    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == static_cast<uint8_t>(immediateValue << 1), "Test #" << testNumber << " failed, incorrect shift left result, expected: '" << AS_HEX(2, static_cast<uint8_t>(immediateValue << 1)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[0xf] == static_cast<uint8_t>(!!(immediateValue & 0x80)), "Test #" << testNumber << " failed, msb flag result, expected: '" << AS_HEX(2, static_cast<uint8_t>(!!(immediateValue & 0x80))) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[0xf]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_SNE_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xe)^ BoostData::random(0, 0xff) ^ BoostData::random(0, 0xff) , testNumber, initAddr, registerIndex0, registerIndex1, registerIndex2, immediateValue0, immediateValue1){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex0, immediateValue0);
    chip8TestInst.LD_IMM(registerIndex1, immediateValue0);
    chip8TestInst.SNE_REG(registerIndex0, registerIndex1);
    uint expected = 2 * initAddr;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for value0, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
    chip8TestInst.LD_IMM(registerIndex2, immediateValue1);
    chip8TestInst.SNE_REG(registerIndex0, registerIndex2);
    expected += (immediateValue0 != immediateValue1 && registerIndex0 != registerIndex2)? 2 : 0;
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << " failed for value1, incorrect program counter result, expected: '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_I, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xfff), testNumber, addrValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_I(addrValue);

    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_iReg == addrValue, "Test #" << testNumber << " failed, incorrect load result, expected: '" << AS_HEX(2, addrValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_iReg) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_JMP_REG, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2) ^ BoostData::random(0, 0xff) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 4), testNumber, initAddr, immediateValue, callAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(0x0, immediateValue);
    chip8TestInst.JMP_REG(2  * callAddr);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == 2 * callAddr + immediateValue, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, 2 * callAddr) << "'; actual: '" << AS_HEX(3, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_RND, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff) ^ BoostData::random(0, INT_MAX), testNumber, registerIndex, immediateValue, seed){
    Chip8Test chip8TestInst(seed);
    std::uniform_int_distribution<> testDist(0, 0xff);
    std::mt19937 testGen(seed);
    for(int i=0; i< NUM_DATA_TESTS; ++i){
        chip8TestInst.RND(registerIndex, immediateValue);
        uint8_t expected = testDist(testGen) & immediateValue;
        BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == expected, "Test #" << testNumber << "failed, incorrect random result, expected '" << AS_HEX(2, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
    }
}

BOOST_DATA_TEST_CASE(Chip8Test_DRW, BoostData::xrange(0, NUM_DATA_TESTS), testNumber){
    // TODO
}

BOOST_DATA_TEST_CASE(Chip8Test_SKP, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0x1f) ^ BoostData::random(0, 0xffff) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, registerIndex, immediateValue, keyValue, initAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_keystates = keyValue;
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.SKP(registerIndex);
    uint16_t expected = 2 * initAddr + ((immediateValue < 0x10 && (keyValue >> immediateValue & 0x1))? 2 : 0);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_SKNP, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0x1f) ^ BoostData::random(0, 0xffff) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, registerIndex, immediateValue, keyValue, initAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_keystates = keyValue;
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.SKNP(registerIndex);
    uint16_t expected = 2 * initAddr + ((immediateValue < 0x10 && (!(keyValue >> immediateValue & 0x1)))? 2 : 0);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expected, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, expected) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_VX_DT, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_dtReg = immediateValue;
    chip8TestInst.LD_VX_DT(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == static_cast<uint8_t>(immediateValue), "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, immediateValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_KP, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0x1f) ^ BoostData::random(0, 0xffff) ^ BoostData::random(0x101, (CHIP8_MAIN_MEM_SIZE - 2) / 2), testNumber, registerIndex, immediateValue, keyValue, initAddr){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.m_keystates = keyValue;
    chip8TestInst.m_programCounter = 2 * initAddr;
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.LD_KP(registerIndex);
    uint8_t expectedRegisterValue = (keyValue)? 0 : immediateValue;
    if(!expectedRegisterValue){
        while(!((keyValue >> expectedRegisterValue) & 0x1)){
            ++expectedRegisterValue;
        }
    }
    uint16_t expectedProgramCounter = 2 * initAddr + ((keyValue)? 2 : 0);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_vRegs[registerIndex] == expectedRegisterValue, "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, expectedRegisterValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_vRegs[registerIndex]) << "'");
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_programCounter == expectedProgramCounter, "Test #" << testNumber << "failed, incorrect program counter result, expected '" << AS_HEX(3, expectedProgramCounter) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_programCounter) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_DT_VX, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.LD_DT_VX(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_dtReg == static_cast<uint8_t>(immediateValue), "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, immediateValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_dtReg) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_ST, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.LD_ST(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_stReg == static_cast<uint8_t>(immediateValue), "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, immediateValue) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_stReg) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_ADD_I, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xfff) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, initAddr, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_I(initAddr);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.ADD_I(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_iReg == static_cast<uint16_t>(immediateValue + initAddr), "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, static_cast<uint16_t>(immediateValue + initAddr)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_iReg) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_SPRT, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xf), testNumber, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.LD_SPRT(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_iReg == static_cast<uint16_t>(5 * immediateValue), "Test #" << testNumber << "failed, incorrect load result, expected '" << AS_HEX(2, static_cast<uint16_t>(5 * immediateValue)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_iReg) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_BCD, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, CHIP8_MAIN_MEM_SIZE - 6) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, 0xff), testNumber, writeAddr, registerIndex, immediateValue){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    chip8TestInst.LD_I(writeAddr);
    chip8TestInst.LD_IMM(registerIndex, immediateValue);
    chip8TestInst.LD_BCD(registerIndex);
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_memory[writeAddr] == static_cast<uint8_t>(immediateValue / 100), "Test #" << testNumber << "failed, incorrect 100's digit load result, expected '" << AS_HEX(2, static_cast<uint8_t>(immediateValue / 100)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_memory[writeAddr]) << "'");
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_memory[writeAddr + 1] == static_cast<uint8_t>((immediateValue % 100) / 10), "Test #" << testNumber << "failed, incorrect 10's digit load result, expected '" << AS_HEX(2, static_cast<uint8_t>((immediateValue % 100) / 10)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_memory[writeAddr + 1]) << "'");
    BOOST_REQUIRE_MESSAGE(chip8TestInst.m_memory[writeAddr + 2] == static_cast<uint8_t>(immediateValue % 10), "Test #" << testNumber << "failed, incorrect 1's digitload result, expected '" << AS_HEX(2, static_cast<uint8_t>(immediateValue % 10)) << "'; actual: '" << AS_HEX(2, chip8TestInst.m_memory[writeAddr + 2]) << "'");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_MEM, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, CHIP8_MAIN_MEM_SIZE - 0x10) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, INT_MAX), testNumber, writeAddr, registerIndex, seed){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    std::mt19937 testGen(seed);
    std::uniform_int_distribution<> testDist(0, 0xff);
    std::vector<uint8_t> testValues;
    for(int i=0; i <= registerIndex; ++i){
        testValues.push_back(static_cast<uint8_t>(testDist(testGen)));
        chip8TestInst.LD_IMM(i, testValues.back());
    }
    chip8TestInst.LD_I(writeAddr);
    chip8TestInst.LD_MEM(registerIndex);

    auto collectionToString = [](auto beg, auto end) -> std::string{
        if(beg != end){
            std::ostringstream res;
            if(std::distance(beg, end))
                std::copy(beg, end - 1, std::ostream_iterator<uint>(res, ", "));
            res << static_cast<uint>(*end);
            return res.str();
        }
        return std::string("");
    };
    BOOST_REQUIRE_MESSAGE(std::equal(testValues.begin(), testValues.end(), std::begin(chip8TestInst.m_memory) + writeAddr), "Test #" << testNumber << "failed, incorrect loaded memory values at <" << writeAddr <<">[" << 0 << "-" << testValues.size() << ") result, expected [" << collectionToString(testValues.cbegin(), testValues.cend() - 1) << "]; actual: [" << collectionToString(chip8TestInst.m_memory.cbegin() + writeAddr, chip8TestInst.m_memory.cbegin() + writeAddr + testValues.size() - 1) << "]");
}

BOOST_DATA_TEST_CASE(Chip8Test_LD_REGS, BoostData::xrange(0, NUM_DATA_TESTS) ^ BoostData::random(0, CHIP8_MAIN_MEM_SIZE - 0x10) ^ BoostData::random(0, 0xe) ^ BoostData::random(0, INT_MAX), testNumber, writeAddr, registerIndex, seed){
    Chip8Test chip8TestInst(std::mt19937::default_seed);
    std::mt19937 testGen(seed);
    std::uniform_int_distribution<> testDist(0, 0xff);
    std::vector<uint8_t> testValues;
    for(int i=0; i <= registerIndex; ++i){
        testValues.push_back(static_cast<uint8_t>(testDist(testGen)));
        chip8TestInst.m_memory[writeAddr + i] =testValues.back();
    }
    chip8TestInst.LD_I(writeAddr);
    chip8TestInst.LD_REGS(registerIndex);

    auto collectionToString = [](auto beg, auto end) -> std::string{
        if(beg != end){
            std::ostringstream res;
            if(std::distance(beg, end))
                std::copy(beg, end - 1, std::ostream_iterator<uint>(res, ", "));
            res << static_cast<uint>(*end);
            return res.str();
        }
        return std::string("");
    };
    BOOST_REQUIRE_MESSAGE(std::equal(testValues.begin(), testValues.end(), std::begin(chip8TestInst.m_memory) + writeAddr), "Test #" << testNumber << "failed, incorrect loaded registers values from memory address <" << writeAddr <<">[" << 0 << "-" << testValues.size() << ") result, expected [" << collectionToString(testValues.cbegin(), testValues.cend() - 1) << "]; actual: [" << collectionToString(chip8TestInst.m_memory.cbegin() + writeAddr, chip8TestInst.m_memory.cbegin() + writeAddr + testValues.size() - 1) << "]");
}
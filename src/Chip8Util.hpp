//
// Created by rmpowell on 10/13/18.
//

#ifndef CHIP8_UTIL_HPP
#define CHIP8_UTIL_HPP

#include <sys/stat.h>

#define CHIP8_UTIL_BIT_WIDTH(type) (8 * sizeof(type))

namespace Chip8{
    namespace Util{

    extern const char* regFile;
    const char* fileExists(const char* t_filePath);

    } // namesapce Util
} // namespace Chip8

#endif // CHIP8_UTIL_HPP

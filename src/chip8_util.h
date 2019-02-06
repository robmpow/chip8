//
// Created by rmpowell on 10/13/18.
//

#ifndef CHIP8_UTIL_H
#define CHIP8_UTIL_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdarg>
#include <sys/stat.h>
#include <chrono>
#include <ctime>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define BIT_WIDTH(type) (8 * sizeof(type))

extern const char* file_typenames[];

extern const char* reg_file;

const char* fileExists(char* file_path);

#endif //CHIP8_UTIL_H

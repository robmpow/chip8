//
// Created by rmpowell on 10/13/18.
//

#ifndef CHIP8_UTIL_H
#define CHIP8_UTIL_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdarg>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifdef DEBUG
#define D(x) x
#else
#define D(x) 
#endif

using namespace std;

void debugMsg(const char* format_str, ...);
void debugMsg(const char* format_str, va_list vargs);
void errorMsg(const char* format_str, ...);
void errorMsg(const char* format_str, va_list vargs);
void okMsg(const char* format_str, ...);
void okMsg(const char* format_str, va_list vargs);
void fatalError(int error_code, const char* format_string, ...);
string build_error_message(const char* format_string, ...);
string build_error_message(const char* format_string, va_list vargs);

template<typename T, typename U>
class mapper{

private:
    T in_clamp_l, in_clamp_u;
    U out_clamp_l, out_clamp_u;
    
public:
    mapper(T i_l, T i_u, U o_l, U o_u);
    typename enable_if<is_integral<U>::value || is_floating_point<U>::value, U>::type map(T value);
    typename enable_if<is_integral<T>::value || is_floating_point<T>::value, T>::type unmap(U value);
};

#endif //CHIP8_UTIL_H

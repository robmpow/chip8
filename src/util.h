//
// Created by rmpowell on 10/13/18.
//

#ifndef CHIP8_UTIL_H
#define CHIP8_UTIL_H

#include <iostream>
#include <string>
#include <cstring>
#include <cstdarg>

#define STRINGIFY(s) #s

#define DBG   33
#define SUCC    32
#define ERR     31

#ifdef DEBUG
#define D(C, ...) do{std::cout << "\033[1;" STRINGIFY(C) "m" << build_error_message(__VA_ARGS__) << "\033[0m";} while(0)
#else
#define D(C, ...)
#endif

void fatalError(int error_code, const char* format_string, ...);
std::string build_error_message(const char* format_string, ...);
std::string build_error_message(const char* format_string, va_list vargs);

void fatalError(int error_code, const char* format_string, ...){
    va_list vargs;
    va_start(vargs, format_string);
    std::cout << "\033[1;31m" << build_error_message(format_string, vargs) << "\033[0m";
    va_end(vargs);
    exit(error_code);
}

std::string build_error_message(const char* format_string, ...) {
    char temp[2048];
    va_list vargs;
    va_start(vargs, format_string);
    vsprintf(temp, format_string, vargs);
    va_end(vargs);
    return std::string(temp);
}

std::string build_error_message(const char* format_string, va_list vargs){
    char temp[2048];
    vsprintf(temp, format_string, vargs);
    va_end(vargs);
    return std::string(temp);
}

template<typename T, typename U>
class mapper{

private:
    T in_clamp_l, in_clamp_u;
    U out_clamp_l, out_clamp_u;
    
public:
    mapper(T i_l, T i_u, U o_l, U o_u){
        in_clamp_l = i_l;
        in_clamp_u = i_u;
        out_clamp_l = o_l;
        out_clamp_u = o_u;
    }

    U map(T val){
        if(val <= in_clamp_l)
            return out_clamp_l;
        if(val >= in_clamp_u)
            return out_clamp_u;
        return (U) (((double) val - in_clamp_l) * (out_clamp_u - out_clamp_l)) / (in_clamp_u - in_clamp_l);
    }

    T unmap(U val){
        if(val <= out_clamp_l)
            return in_clamp_l;
        if(val >= out_clamp_u)
            return in_clamp_u;
        return (T) (((double) val - out_clamp_l) * (in_clamp_u - in_clamp_l)) / (out_clamp_u - out_clamp_l);
    }
};

#endif //CHIP8_UTIL_H

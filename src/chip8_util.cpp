#include "chip8_util.h"

static void msg(const char* color_str, const char* format_str, va_list vargs){
    std::cout << color_str;
    vprintf(format_str, vargs);
    std::cout << ANSI_COLOR_RESET;
}

void debugMsg(const char* format_str, ...){
    va_list vargs;
    va_start(vargs, format_str);
    msg(ANSI_COLOR_YELLOW, format_str, vargs);
    va_end(vargs);
}

void debugMsg(const char* format_str, va_list vargs){
    msg(ANSI_COLOR_YELLOW, format_str, vargs);
}

void errorMsg(const char* format_str, ...){
    va_list vargs;
    va_start(vargs, format_str);
    msg(ANSI_COLOR_RED, format_str, vargs);
    va_end(vargs);
}

void errorMsg(const char* format_str, va_list vargs){
    msg(ANSI_COLOR_RED, format_str, vargs);
}

void okMsg(const char* format_str, ...){
    va_list vargs;
    va_start(vargs, format_str);
    msg(ANSI_COLOR_GREEN, format_str, vargs);
    va_end(vargs);
}

void okMsg(const char* format_str, va_list vargs){
    msg(ANSI_COLOR_GREEN, format_str, vargs);
}

void fatalError(int error_code, const char* format_string, ...){
    va_list vargs;
    va_start(vargs, format_string);
    errorMsg(format_string, vargs);
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
mapper<T, U>::mapper(T i_l, T i_u, U o_l, U o_u){
    in_clamp_l = i_l;
    in_clamp_u = i_u;
    out_clamp_l = o_l;
    out_clamp_u = o_u;
}

template<typename T, typename U>
U mapper<T, U>::map(T val){
    if(val <= in_clamp_l)
        return out_clamp_l;
    if(val >= in_clamp_u)
        return out_clamp_u;
    return (U) (((double) val - in_clamp_l) * (out_clamp_u - out_clamp_l)) / (in_clamp_u - in_clamp_l);
}

template<typename T, typename U>
T mapper<T, U>::unmap(U val){
    if(val <= out_clamp_l)
        return in_clamp_l;
    if(val >= out_clamp_u)
        return in_clamp_u;
    return (T) (((double) val - out_clamp_l) * (in_clamp_u - in_clamp_l)) / (out_clamp_u - out_clamp_l);
}
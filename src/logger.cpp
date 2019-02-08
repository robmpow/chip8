#include "logger.hpp"
#include "chip8_util.h"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace logger{

    std::ostream&(* const endl)(std::ostream&) = static_cast<std::ostream&(* const)(std::ostream&)>(std::endl);

    std::array<std::string, 6> log_level_color  = { ANSI_COLOR_RED, ANSI_COLOR_MAGENTA, ANSI_COLOR_YELLOW,
                                                    ANSI_COLOR_BLUE, ANSI_COLOR_CYAN, ANSI_COLOR_GREEN};

    std::array<std::string, 6> log_level_string = { "fatal", "error", "warning",
                                                    "debug", "trace", "info",};

    int8_t log_all = log_info;

    int8_t stringToLogLevel(std::string level_string){
        std::transform(level_string.begin(), level_string.end(), level_string.begin(), ::tolower);
        if(!level_string.compare("all")){
            return log_all;
        }
        for(uint i = 0; i < log_level_string.size(); i++){
            if(!level_string.compare(log_level_string[i])){
                return i;
            }
        }
        return -1;
    }
}
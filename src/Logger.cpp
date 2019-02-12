#include "Logger.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace Logger{

std::ostream&(* const endl)(std::ostream&) = static_cast<std::ostream&(* const)(std::ostream&)>(std::endl);

std::array<const std::string, 6> logLevelColors = { LOGGER_ANSI_COLOR_RED,  LOGGER_ANSI_COLOR_MAGENTA,  LOGGER_ANSI_COLOR_YELLOW,
                                                    LOGGER_ANSI_COLOR_BLUE, LOGGER_ANSI_COLOR_CYAN,     LOGGER_ANSI_COLOR_GREEN};

std::array<const std::string, 6> logLevelStrings = {"fatal", "error", "warning",
                                                    "debug", "trace", "info",};

int8_t logAll = LogInfo;

int8_t stringToLogLevel(std::string t_levelString){
    std::transform(t_levelString.begin(), t_levelString.end(), t_levelString.begin(), ::tolower);
    if(!t_levelString.compare("all")){
        return logAll;
    }
    for(uint i = 0; i < logLevelStrings.size(); i++){
        if(!t_levelString.compare(logLevelStrings[i])){
            return i;
        }
    }
    return -1;
}

} // namespace logger
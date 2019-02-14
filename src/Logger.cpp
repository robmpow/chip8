#include "Logger.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace Logger{

std::ostream&(* const endl)(std::ostream&) = static_cast<std::ostream&(* const)(std::ostream&)>(std::endl);

std::array<const std::string, 6> logLevelColors = { LOGGER_ANSI_COLOR_RED,  LOGGER_ANSI_COLOR_MAGENTA,  LOGGER_ANSI_COLOR_YELLOW,
                                                    LOGGER_ANSI_COLOR_BLUE, LOGGER_ANSI_COLOR_CYAN,     LOGGER_ANSI_COLOR_GREEN};

static std::array<const std::string, 6> logLevelStrings = {"fatal", "error", "warning",
                                                           "debug", "trace", "info",};

const std::string& getStringFromLevel(LogLevel t_level){
    return logLevelStrings[t_level];
}

const std::string& getLevelColorString(LogLevel t_level){
    return logLevelColors[t_level];
}

LogLevel getLevelFromString(std::string t_string){
    std::transform(t_string.begin(), t_string.end(), t_string.begin(), ::tolower);
    if(!t_string.compare("all")){
        return LogAll;
    }
    for(uint i = 0; i < logLevelStrings.size(); i++){
        if(!t_string.compare(logLevelStrings[i])){
            return static_cast<LogLevel>(i);
        }
    }
    throw LoggerException("'" + t_string + "' does not name a log level");
}

LogLevel LogAll = LogInfo;

LoggerException::LoggerException(std::string t_exceptMsg) noexcept{
    m_exceptMsg = t_exceptMsg;
}

const char* LoggerException::what() const noexcept{
    return m_exceptMsg.c_str();
}

} // namespace logger
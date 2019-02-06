#include "logger.hpp"
#include "chip8_util.h"

#include <iostream>
#include <sstream>

namespace logger{

    std::ostream&(*endl)(std::ostream&) = static_cast<std::ostream&(*)(std::ostream&)>(std::endl);

    std::array<std::string, 6> log_level_color  = { ANSI_COLOR_RED, ANSI_COLOR_MAGENTA, ANSI_COLOR_YELLOW,
                                                    ANSI_COLOR_BLUE, ANSI_COLOR_CYAN, ANSI_COLOR_GREEN};

    std::array<std::string, 6> log_level_string = { "Fatal", "Error", "Warning",
                                                    "Debug", "Trace", "Info",};
}
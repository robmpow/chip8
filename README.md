# Chip8 Emulator

This is a chip8 emulator I wrote in C++ using SDL2 for display, sound and input.

## Compilation

The emulator can be built using the provided makefile. Note this project is targeted at Linux and uses Posix functions that do not work on other operating systems. The built binary is placed into the bin directory.

## Usage

chip8 [ROM File] [Options]
    -r, --res=WxH           set the display resolution to WxH
    -c, --config=FILE       set emulator ini file to read configuration from. 
        --log_level=LEVEL   set logger level, any message with level below LEVEL is ignored;
                            LEVEL can be 'all', 'fatal', 'error', 'warning', 'debug', 'trace'
                            'info'\n"
        --log_file[=FILE]   enables logging and sets the desination file; if FILE is omitted
                            the default log file is used: ./log/chip8_MM-DD-YYYY_HH:MM:SS.log
        --log_enable[=BOOL] sets logger enable to BOOL; BOOL can be 'true' | '1' (default),
                            'false' | '0'
    -h, --help              Prints this usage message then exits.


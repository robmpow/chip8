/* 
 * Chip8 main
 * Author: Robert Powell 
 */

#include <iostream>
#include <string>

#include <cstdint>
#include <getopt.h>
#include <unordered_map>
#include <set>
#include <utility>
#include <chrono>
#include <unistd.h>
#include <functional>
#include <iomanip>

#include "chip8.h"
#include "chip8_util.h"
#include "ini_reader.h"
#include "key_handler_impl.h"
#include "chip8_emulator.h"
#include "logger.hpp"
#include "logger_impl.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static const option opts[] =   {{"res",     required_argument,  0,  'r'},
                                {"log",     optional_argument,  0,  'l'},
                                {"help",    no_argument,        0,  'h'},
                                {0,         0,                  0,  0}};

static const char usage[] = "usage: %s rom_file [-r | -res widthxheight] [-l | --log filepath] [-h | --help]\n"
                            "\t-r | --res wxh: sets the starting display resolution width to w and height to h.\n"
                            "\t-l | --log filepath: enables logging, if filepath is provided the logfile is set to filepath.\n";

namespace arg = std::placeholders;

logger::logger<logger::logger_interface_impl> chip8_logger;

int main(int argc, char** argv){

    std::pair<int, int> resolution;
    std::pair<SDL_Color, SDL_Color> palette;
    std::unordered_map<key_enum, key_action> bind_map;

    union{
        bitfield<uint8_t, 0, 1> log;
        bitfield<uint8_t, 0, 8> all;
    } flags;

    flags.all = 0;

    int opt;
    std::string rom_path;
    opterr = 0;
    while((opt = getopt_long(argc, argv, "r:h", opts, NULL)) != -1){
        switch(opt){
            case 'r':
                if(optarg){
                    char* split;
                    resolution.first = strtoul(optarg, &split, 10);
                    if(*split){
                        resolution.second = strtoul(split + 1, NULL, 10);
                    }
                    else{
                        chip8_logger.log<logger::log_fatal>("Error: [-r | -res wxh ]: invalid screen resolution format.\n");
                        exit(-1);
                    }
                    if(!resolution.first || !resolution.second){
                        chip8_logger.log<logger::log_fatal>("Error: [-r | -res wxh ]: invalid screen resolution format.\n");
                        exit(-1);
                    }
                }
                else{
                    chip8_logger.log<logger::log_fatal>("Error: [-r | --res wxh]: missing screen resolution argument.\n");
                    exit(-1);
                }
                break;
            case 'h':
                printf(usage, argv[0]);
                exit(0);
            case 'l':
                flags.log |= 1;
                break;
            case '?':
                chip8_logger.log<logger::log_fatal>("Error: Unkownn option '", static_cast<char>(optopt), "'.\n");
                printf(usage, argv[0]);
                exit(1);
            default:
                break;
        }
    }

    while(optind < argc){
        const char* type;
        if((type = fileExists(argv[optind])) != reg_file){
            if(type){
                chip8_logger.log<logger::log_fatal>("Error: ", argv[optind], ": File type ", type, " is not supported.\n");
                exit(-1);
            }
            chip8_logger.log<logger::log_fatal>("Error: ", argv[optind], ": file not found.\n");
            exit(-1);
        }
        rom_path = argv[optind];
        optind++;
    }
    if(rom_path.empty()){
        chip8_logger.log<logger::log_fatal>("Error: No input file path to chip8 rom.\n");
        exit(-1);
    }

    if(flags.log){
        chip8_logger.set_log_file("chip8" + rom_path);
    }

    try{

        const std::unordered_map<std::string, key_action> chip8_key_binds({ {"key_ch8_0",       KEY_CH8_0},
                                                                            {"key_ch8_1",       KEY_CH8_1},
                                                                            {"key_ch8_2",       KEY_CH8_2},
                                                                            {"key_ch8_3",       KEY_CH8_3},
                                                                            {"key_ch8_4",       KEY_CH8_4},
                                                                            {"key_ch8_5",       KEY_CH8_5},
                                                                            {"key_ch8_6",       KEY_CH8_6},
                                                                            {"key_ch8_7",       KEY_CH8_7},
                                                                            {"key_ch8_8",       KEY_CH8_8},
                                                                            {"key_ch8_9",       KEY_CH8_9},
                                                                            {"key_ch8_a",       KEY_CH8_A},
                                                                            {"key_ch8_b",       KEY_CH8_B},
                                                                            {"key_ch8_c",       KEY_CH8_C},
                                                                            {"key_ch8_d",       KEY_CH8_D},
                                                                            {"key_ch8_e",       KEY_CH8_E},
                                                                            {"key_ch8_f",       KEY_CH8_F},
                                                                            {"key_emu_pause",   KEY_EMU_PAUSE},
                                                                            {"key_emu_reset",   KEY_EMU_RESET},});

        ini_reader config("res/config.ini");

        resolution.first = config.getInt("Display", "disp_width", 640);
        resolution.second = config.getInt("Display", "disp_height", 480);

        int fg_raw = config.getInt("Display", "disp_fg_color", 0x000000);
        int bg_raw = config.getInt("Display", "disp_bg_color", 0xFFFFFF);

        palette.second.r = (bg_raw >> 16) & 0xFF;
        palette.second.g = (bg_raw >> 8) & 0xFF;
        palette.second.b = bg_raw & 0xFF;
        palette.second.a = 0xFF;

        palette.first.r = (fg_raw >> 16) & 0xFF;
        palette.first.g = (fg_raw >> 8) & 0xFF;
        palette.first.b = fg_raw & 0xFF;
        palette.first.a = 0xFF;

        auto chip8_ini_binds = config.getHeaderValues("Keys");

        for(auto ini_binds_it = chip8_ini_binds.begin(); ini_binds_it != chip8_ini_binds.end(); ini_binds_it++){
            if(chip8_key_binds.count(ini_binds_it->first)){ 
                key_enum bind = {0,KMOD_NONE};

                std::size_t ind;
                if((ind = ini_binds_it->second.find('+')) != std::string::npos){
                    if(ind == 0 || ind == ini_binds_it->second.size()){
                        chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', key or modifier '", ini_binds_it->second, "' not recognized\n", logger::endl);
                        exit(-1);
                    }

                    std::string bind_key0 = ini_binds_it->second.substr(ini_binds_it->second.find_first_not_of(" "),            ini_binds_it->second.find_last_not_of(" ", ind) - 1);
                    std::string bind_key1 = ini_binds_it->second.substr(ini_binds_it->second.find_first_not_of(" ", ind + 1),   ini_binds_it->second.find_last_not_of(" ") - 1);

                    if((bind.modifier = lookUpSDLKeymod(bind_key0)) != KMOD_NONE){
                        if((bind.key = lookUpSDLKeycode(bind_key1)) == SDLK_UNKNOWN){
                            chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', key or modifier '", bind_key1, "' not recognized\n", logger::endl);
                            exit(-1);
                        }
                    
                    }
                    else if((bind.key = lookUpSDLKeycode(bind_key0)) != SDLK_UNKNOWN){
                        if((bind.modifier = lookUpSDLKeymod(bind_key1)) == KMOD_NONE){
                            chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', key or modifier '", bind_key1, "' not recognized\n", logger::endl);  
                            exit(-1);

                        }
                        else{
                            chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', key or modifier '", bind_key0, "' not recognized\n", logger::endl);  
                            exit(-1);
                        }
                    }
                    else{
                        if((bind.key = lookUpSDLKeycode(ini_binds_it->second)) == SDLK_UNKNOWN){
                            chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', key or modifier '", ini_binds_it->second, "' not recognized", logger::endl);
                            exit(-1);  
                        }
                    }

                    bind_map.emplace(bind, chip8_key_binds.find(ini_binds_it->first)->second);
                }
            }
            else{
                chip8_logger.log<logger::log_fatal>("Error: Invalid keybind '", ini_binds_it->first, "=", ini_binds_it->second, "', bind '", ini_binds_it->second, "' not recognized", logger::endl);
                exit(-1);
            }
        }
    }
    catch(std::string error){
        chip8_logger.log<logger::log_fatal>("INI file parse error: ", error, logger::endl);
        exit(-1);  
    }

    std::cout << std::endl;

    SDL_Log("Screen res set width: %d, height: %d.\n", resolution.first, resolution.second);
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1){
        chip8_logger.log<logger::log_fatal>("SDL Error:", SDL_GetError(), logger::endl);
    }

    if(TTF_Init() == -1){
        
    }

    chip8_logger.log<logger::log_info>("Resolution: ", resolution.first, "x", resolution.second, logger::endl);
    chip8_logger.log<logger::log_info>("Rom: ", rom_path, logger::endl);
    chip8_logger.log<logger::log_info>("Paletter: fg=0x", std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.r),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.g),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.b),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.a),
                                                " bg=0x", std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.r),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.g),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.b),
                                                          std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.a), logger::endl);

    chip8_emulator emu(resolution, palette, rom_path, bind_map);

    emu.run();

    TTF_Quit();

    SDL_Quit();

    return(0);

}
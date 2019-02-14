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
#include <ctype.h>
#include <algorithm>

#include "Chip8.hpp"
#include "Chip8Util.hpp"
#include "IniReader.hpp"
#include "KeyHandler.hpp"
#include "Chip8Emulator.hpp"
#include "LoggerImpl.hpp"

#include <SDL2/SDL.h>

static const option long_opts[] =   {{"resolution",  required_argument,  0,  'r'},
                                     {"log_enable",  optional_argument,  0,  0},
                                     {"log_level",   required_argument,  0,  0},
                                     {"log_file",    optional_argument,  0,  0},
                                     {"config",      required_argument,  0,  'c'},
                                     {"help",        no_argument,        0,  'h'},
                                     {0,             0,                  0,  0}};

static const char usage[] = "[ROM File] [Options]\n"
                            "\t-r, --res=WxH           set the display resolution to WxH\n"
                            "\t-c, --config=FILE       set emulator ini file to read configuration from.\n" 
                            "\t    --log_level=LEVEL   set logger level, any message with level below LEVEL is ignored;\n"
                            "\t                        LEVEL can be 'all', 'fatal', 'error', 'warning', 'debug', 'trace'\n"
                            "\t                        'info'\n"
                            "\t    --log_file[=FILE]   enables logging and sets the desination file; if FILE is omitted\n"
                            "\t                        the default log file is used: ./log/chip8_MM-DD-YYYY_HH:MM:SS.log\n"
                            "\t    --log_enable[=BOOL] sets logger enable to BOOL; BOOL can be 'true' | '1' (default),\n"
                            "\t                        'false' | '0'\n"
                            "\t-h, --help              Prints this usage message then exits.";

namespace arg = std::placeholders;

int main(int argc, char** argv){

    std::pair<int, int> resolution = {0, 0};
    std::pair<SDL_Color, SDL_Color> palette;
    std::unordered_map<Chip8::KeyHandler::KeyPair, Chip8::KeyHandler::KeyAction> bindMap;

    union{
        Bitfield::Bitfield<uint8_t, 0, 1> log;
        Bitfield::Bitfield<uint8_t, 1, 1> force_log;
        Bitfield::Bitfield<uint8_t, 2, 1> logFile;
        Bitfield::Bitfield<uint8_t, 0, 8> all;
    } flags;

    flags.all = 0;

    int opt, longopt_ind = 0;
    std::string romPath;
    std::string logFile;
    std::string configFile = "res/config.ini";
    std::regex resolutionRegex("(\\d*)x(\\d*)");
    std::cmatch matchRes;
    opterr = 0;
    while((opt = getopt_long(argc, argv, "r:c:h", long_opts, &longopt_ind)) != -1){
        switch(opt){
            case 'r':
                if(std::regex_match(optarg, matchRes, resolutionRegex)){
                    resolution.first = std::strtoul(matchRes[1].str().c_str(), nullptr, 10);
                    resolution.second = std::strtoul(matchRes[2].str().c_str(), nullptr, 10);
                }
                else{
                    std::cerr << argv[0] << ": Error option '-r | --resolution' argument '" << optarg << "' is not recognized\nTry '" << argv[0] << " --help for more information" << std::endl;
                    exit(-1);
                }
                break;
            case 'c':
                configFile = std::string(optarg);
            case 'h':
                std::cout << "Usage: " << argv[0] << usage << std::endl;
                exit(0);
            case 0:
                switch(longopt_ind){
                    case 1:
                        // log_enable
                        if(optarg){
                            std::string opt_string(optarg);
                            std::transform(opt_string.begin(), opt_string.end(), opt_string.begin(), ::tolower);
                            if(!opt_string.compare("1")){
                                flags.force_log = 1;
                            }
                            else if(!opt_string.compare("0")){
                                flags.force_log = 0;
                            }
                            else if(!opt_string.compare("true")){
                                flags.force_log = 1;
                            }
                            else if(!opt_string.compare("false")){
                                flags.force_log = 0;
                            }
                            else{
                                std::cerr << argv[0] << ": Error option '--log_enable' argument '" << optarg << "' is not recognized\nTry '" << argv[0] << " --help for more information" << std::endl;
                                exit(-1);
                            }
                        }
                        else{
                            flags.log = 1;
                        }
                        break;
                    case 2:
                        // log_level
                        try{
                            chip8Logger.setLogLevel(Logger::getLevelFromString(optarg));
                        }
                        catch(const Logger::LoggerException& except){
                            std::cerr << argv[0] << ": Error option '--log_level' argument " << except.what() << "\nTry '" << argv[0] << " --help for more information" << std::endl;
                            exit(-1);
                        }
                        break;
                    case 3:
                        // logFile
                        flags.logFile = 1;
                        logFile = std::string(optarg);
                        break;
                }
                break;
            case 'l':
                flags.log |= 1;
                break;
            case 'f':
                flags.log |= 1;
                flags.logFile = 1;

            case '?':
                std::cerr << argv[0] << ": Error unknown option '" << static_cast<char>(optopt) << "'" << std::endl;
                std::cout << "Usage: " << argv[0] << " " << usage << std::endl;
                exit(-1);
            default:
                break;
        }
    }

    while(optind < argc){
        const char* type;
        if((type = Chip8::Util::fileExists(argv[optind])) != Chip8::Util::regFile){
            if(type){
                std::cerr << argv[0] << ": Error file '" << argv[optind] << "' is not supported" << std::endl;
                exit(-1);
            }
            std::cerr << argv[0] << ": Error file '" << argv[optind] <<  "' not found" << std::endl;
            exit(-1);
        }
     romPath = argv[optind];
        optind++;
    }
    if(romPath.empty()){
        std::cerr << "Error: No input file path to chip8 rom." << std::endl;
        exit(-1);
    }

    // Init logger if enabled
    if(flags.logFile && !logFile.empty()){
        chip8Logger.setLogEnable(true);
        chip8Logger.setLogFile(logFile);
    }
    else if(flags.logFile){
        chip8Logger.setLogEnable(true);
        chip8Logger.setDefaultLogFile();
    }

    try{
        IniReader config(configFile);

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

        auto chip8IniBinds = config.getHeaderValues("Keys");

        for(auto iniBindsIt = chip8IniBinds.begin(); iniBindsIt != chip8IniBinds.end(); iniBindsIt++){
            Chip8::KeyHandler::KeyAction bindAction = Chip8::KeyHandler::getActionFromName(iniBindsIt->first);
            Chip8::KeyHandler::KeyPair bindKeys = {0,KMOD_NONE};

            std::size_t ind;
            if((ind = iniBindsIt->second.find("//")) != std::string::npos){
                if(ind == 0 || ind == iniBindsIt->second.size() - 2){
                    std::cerr << "Error: Invalid keybind '" << iniBindsIt->first << "=" <<  iniBindsIt->second << "', key or modifier '" << iniBindsIt->second << "' not recognized\n" << std::endl;
                    exit(-1);
                }

                std::string bind_key0 = iniBindsIt->second.substr(iniBindsIt->second.find_first_not_of(" "),            iniBindsIt->second.find_last_not_of(" ", ind - 2) + 1);
                std::string bind_key1 = iniBindsIt->second.substr(iniBindsIt->second.find_first_not_of(" ", ind + 2),   iniBindsIt->second.find_last_not_of(" ") + 1);

                if((bindKeys.modifier = Chip8::KeyHandler::getKmodFromName(bind_key0)) != KMOD_NONE){
                    if((bindKeys.key = SDL_GetKeyFromName(bind_key1.c_str())) == SDLK_UNKNOWN){
                        std::cerr << "Error: Invalid keybind '" << iniBindsIt->first << "=" << iniBindsIt->second << "', key or modifier '" << bind_key1 << "' not recognized\n" << std::endl;
                        exit(-1);
                    }
                
                }
                else if((bindKeys.key = SDL_GetKeyFromName(bind_key0.c_str())) != SDLK_UNKNOWN){
                    if((bindKeys.modifier = Chip8::KeyHandler::getKmodFromName(bind_key1)) == KMOD_NONE){
                        std::cerr << "Error: Invalid keybind '" << iniBindsIt->first << "=" << iniBindsIt->second << "' << key or modifier '" << bind_key1 << "' not recognized\n" << std::endl;  
                        exit(-1);
                    }
                }
                else{
                    if((bindKeys.key = SDL_GetKeyFromName(iniBindsIt->second.c_str())) == SDLK_UNKNOWN){
                        std::cerr << "Error: Invalid keybind '" << iniBindsIt->first << "=" << iniBindsIt->second << "' << key or modifier '" << iniBindsIt->second << "' not recognized" << std::endl;
                        exit(-1);  
                    }
                }

                bindMap.emplace(bindKeys, bindAction);
            }
            else if(SDL_GetKeyFromName(iniBindsIt->second.c_str()) != SDLK_UNKNOWN){
                bindMap.emplace(bindKeys, bindAction);
            }
            else{
                std::cerr << "Error: Invalid keybind '" << iniBindsIt->first << "=" << iniBindsIt->second << "' << key '" << iniBindsIt->second << "' not recognized" << std::endl;
                exit(-1);  
            }
        }
    }
    catch(const std::string& error){
        std::cerr << "Ini file: " << error << std::endl;
        exit(-1);  
    }
    catch(const IniReaderException& exception){
        std::cerr << "Ini file: " << exception.what() << std::endl;
        exit(-1);
    }

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1){
        std::cerr << "SDL Error:" << SDL_GetError() << std::endl;
    }

    chip8Logger.log<Logger::LogTrace>("Resolution: ", resolution.first, "x", resolution.second, Logger::endl);
    chip8Logger.log<Logger::LogTrace>("Rom: ", romPath, Logger::endl);
    chip8Logger.log<Logger::LogTrace>("Palette: fg=0x", std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.r),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.g),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.b),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.first.a),
                                              " bg=0x", std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.r),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.g),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.b),
                                                        std::hex, std::setw(2), std::setfill('0'), static_cast<int>(palette.second.a), Logger::endl);


    Chip8::Emulator emulator(resolution, palette, romPath, bindMap, true);

    emulator.run();

    SDL_Quit();

    return(0);

}
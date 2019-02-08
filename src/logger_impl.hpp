#pragma once

#include <algorithm>
#include <regex>

#include "logger.hpp"

#define LOG(level)      chip8_logger.log<level>
#define LOG_FATAL       chip8_logger.log< logger::log_fatal >
#define LOG_ERROR       chip8_logger.log< logger::log_error >
#define LOG_WARNING     chip8_logger.log< logger::log_warning >
#define LOG_DEBUG       chip8_logger.log< logger::log_debug >
#define LOG_TRACE       chip8_logger.log< logger::log_trace >
#define LOG_INFO        chip8_logger.log< logger::log_info >
#define SET_LOG_LEVEL   chip8_logger.set_log_level

namespace logger{

    class logger_interface_impl : public logger_interface{

        protected:
            std::ofstream log_stream;
            uint32_t log_line = 0;
            const std::regex color_regex;

            std::string get_log_heading(log_level ll, bool colored){
                std::string heading = "[";
                std::time_t time = std::time(NULL);
                heading.append(std::ctime(&time));
                heading.erase(--heading.end());
                if(colored)
                    heading.append("] [" +  std::to_string(log_line) + "] [" + log_level_color[ll] + log_level_string[ll] + ANSI_COLOR_RESET + "] ");
                else
                    heading.append("] [" +  std::to_string(log_line) + "] [" + log_level_string[ll] + "] ");
                return heading;
            }

        public:

            logger_interface_impl(): color_regex(""){}

            void open_ostream(){
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string log_file(std::ctime(&time));
                std::replace(log_file.begin(), log_file.end(), ' ', '_');
                log_file.insert(0, "./log/chip8_");
                log_file.erase(--log_file.end());
                log_file.append(".log");
                log_stream.open(log_file, std::fstream::out);
                if(!log_stream.is_open()){
                    throw(std::runtime_error("LOGGER: Could not open output stream"));
                }
            }
            void open_ostream(std::string file_path){
                log_stream.open(file_path, std::fstream::out);
                if(!log_stream.is_open()){
                    throw(std::runtime_error("LOGGER: Could not open output stream"));
                }
            }

            void write(log_level ll, std::string log_msg){
                if(log_stream.is_open()){
                    log_stream << get_log_heading(ll, false) << log_msg;
                }
                std::cout << get_log_heading(ll, true) << log_msg;
            }

            void close_ostream(){
                if(log_stream.is_open()){
                    log_stream.close();
                }
            }

            ~logger_interface_impl(){
                if(log_stream.is_open()){
                    log_stream.close();
                }
            }
    };
}

extern logger::logger<logger::logger_interface_impl> chip8_logger;
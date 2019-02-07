#pragma once

#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <memory>
#include <array>
#include <sstream>

#include "chip8_util.h"


namespace logger {  

    extern std::ostream&(*endl)(std::ostream&);

    class logger_interface{
        public:
            virtual void open_ostream(std::string base_name) = 0;
            virtual void write(std::string log_msg) = 0;
            virtual void close_ostream() = 0;
            virtual ~logger_interface() {};
    };

    enum log_level{
        log_fatal = 0,
        log_error,
        log_warning,
        log_debug,
        log_trace,
        log_info
    };

    extern std::array<std::string, 6> log_level_string;

    extern std::array<std::string, 6> log_level_color;

    template<typename T>
    class logger{
        protected:
            log_level level;    
            T* log_interface = NULL;
            static uint32_t log_line;
            bool color = true;

            std::string get_log_heading(log_level ll){
                std::string heading = "[";
                std::time_t time = std::time(NULL);
                heading.append(std::ctime(&time));
                heading.erase(--heading.end());
                heading.append("] [" +  std::to_string(log_line) + "] [" + log_level_color[ll] + log_level_string[ll] + ANSI_COLOR_RESET + "] ");
                return heading;
            }

            void write(std::stringstream& ss){
                log_interface->write(ss.str());
                log_line++;
            }

            template<typename Next, typename...Others>
            void write(std::stringstream& ss, Next next, Others...others){
                ss << next;
                write(ss, others...);
            }

            template<typename Next, typename...Others>
            void write(Next next, Others...others){
                std::stringstream ss;
                ss << next;
                write(ss, others...);
            }

        public:
            logger() : level(log_info){
                log_interface = new T;
            }

            logger(log_level ll) : level(ll){
                log_interface = new T;
            }

            logger(std::string base_file_name) : level(log_info){
                log_interface = new T;
                log_interface->open_ostream(base_file_name);
            }

            logger(log_level ll, std::string base_file_name) : level(ll){
                log_interface = new T;
                log_interface->open_ostream(base_file_name);
            }

            void set_log_file(std::string file_name){
                log_interface->close_ostream();
                log_interface->open_ostream(file_name);
            }

            void set_log_level(log_level ll){
                level = ll;
            }

            template<log_level L, typename...Args>
            void log(Args...args){
                if(L <= level){
                    log_interface->write(get_log_heading(L));
                }
                write(args...);
            }

            ~logger(){
                if(log_interface){
                    log_interface->close_ostream();
                    delete log_interface;
                }
            }
    };

    template<typename T>
    uint32_t logger<T>::log_line = 0;
}


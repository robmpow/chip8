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

    extern std::ostream&(* const endl)(std::ostream&);

    enum log_level{
        log_fatal = 0,
        log_error,
        log_warning,
        log_debug,
        log_trace,
        log_info
    };

    class logger_interface{
        public:
            virtual void open_ostream() = 0;
            virtual void open_ostream(std::string file_path) = 0;
            virtual void write(log_level ll, std::string log_msg) = 0;
            virtual void close_ostream() = 0;
            virtual ~logger_interface() {};
    };

    extern int8_t log_all;

    extern std::array<std::string, 6> log_level_string;
    extern std::array<std::string, 6> log_level_color;

    int8_t stringToLogLevel(std::string);

    template<typename T>
    class logger{
        protected:
            // static log_level level;    
            // T* log_interface = NULL;
            // static uint32_t log_line;
            // static bool color_enable;
            // static bool log_enable;

            log_level level;    
            T* log_interface = NULL;
            uint32_t log_line;
            bool color_enable;
            bool log_enable;

            std::string build_log_msg(std::stringstream& ss){
                return ss.str();
            }

            template<typename Next, typename...Others>
            std::string build_log_msg(std::stringstream& ss, Next next, Others...others){
                ss << next;
                return build_log_msg(ss, others...);
            }

            template<typename Next, typename...Others>
            std::string build_log_msg(Next next, Others...others){
                std::stringstream ss;
                ss << next;
                return build_log_msg(ss, others...);
            }

        public:
            logger(){
                log_interface = new T;
            }

            logger(log_level ll){
                level = ll;
                log_interface = new T;
            }

            logger(std::string base_file_name){
                log_interface = new T;
                log_interface->open_ostream(base_file_name);
            }

            logger(log_level ll, std::string base_file_name){
                level = ll;
                log_interface = new T;
                log_interface->open_ostream(base_file_name);
            }

            void set_log_file_default(){
                log_interface->close_ostream();
                log_interface->open_ostream();
            }

            void set_log_file(std::string file_name){
                log_interface->close_ostream();
                log_interface->open_ostream(file_name);
            }

            void set_log_level(log_level ll){
                level = ll;
            }

            void set_log_enable(bool enable){
                this->enable = enable;
            }

            template<log_level L, typename...Args>
            void log(Args...args){
                if(L <= level){
                    log_interface->write(L, build_log_msg(args...));
                }
            }

            ~logger(){
                if(log_interface){
                    log_interface->close_ostream();
                    delete log_interface;
                }
            }
    };

    // template<typename T>
    // log_level logger<T>::level = log_warning;
    
    // template<typename T>
    // uint32_t logger<T>::log_line = 0;
    
    // template<typename T>
    // bool logger<T>::color_enable = true;

    // template<typename T>
    // bool logger<T>::log_enable = true;
}


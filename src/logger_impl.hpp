#pragma once

#include "logger.hpp"

namespace logger{

    class logger_interface_impl : public logger_interface{

        protected:
            std::ofstream log_stream;

        public:
            void open_ostream(std::string base_name){
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    std::string log_file(std::ctime(&time));
                    log_file.replace(log_file.begin(), log_file.end(), ' ', '_');
                    log_file.insert(0, base_name);
                    log_file.append(".log");
                    log_stream.open(log_file);
                    if(!log_stream.is_open()){
                        throw(std::runtime_error("LOGGER: Could not open output stream"));
                    }
            }

            void write(std::string log_msg){
                if(log_stream){
                    log_stream << log_msg;
                }
                std::cout << log_msg;
            }

            void close_ostream(){
                if(log_stream){
                    log_stream.close();
                }
            }

            ~logger_interface_impl(){
                if(log_stream){
                    log_stream.close();
                }
            }
    };
}
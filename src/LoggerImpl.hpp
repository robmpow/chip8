#ifndef LOGGER_LOGGER_INTERFACE_IMPL_H
#define LOGGER_LOGGER_INTERFACE_IMPL_H

#include <algorithm>
#include <ctime>
#include <regex>

#include "Logger.hpp"

namespace Logger{

class LoggerInterfaceImpl : public LoggerInterface{

    protected:
        std::ofstream m_logStream;
        uint32_t m_logLine = 0;

        std::string getLogHeading(LogLevel t_level, bool t_colored){
            std::string heading = "[";
            std::time_t time = std::time(NULL);
            heading.append(std::ctime(&time));
            heading.erase(--heading.end());
            if(t_colored)
                heading.append("] [" +  std::to_string(m_logLine) + "] [" + getLevelColorString(t_level) + getStringFromLevel(t_level) + LOGGER_ANSI_COLOR_RESET + "] ");
            else
                heading.append("] [" +  std::to_string(m_logLine) + "] [" + getStringFromLevel(t_level) + "] ");
            return heading;
        }

    public:
        void openOstream(){
            std::time_t rawTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            tm* localTime = localtime(&rawTime);
            char fileNameBuffer[20];
            std::strftime(fileNameBuffer, sizeof(fileNameBuffer), "%m-%d-%Y_%H:%M:%S", localTime);
            std::string logFile(fileNameBuffer);
            logFile.insert(0, "./log/chip8_");
            logFile.append(".log");
            m_logStream.open(logFile, std::fstream::out);
            std::cout << logFile << std::endl;
            if(!m_logStream.is_open()){
                throw(LoggerException("LOGGER: Could not open output stream"));
            }
        }
        void openOstream(const std::string& t_filePath){
            m_logStream.open(t_filePath, std::fstream::out);
            if(!m_logStream.is_open()){
                throw(LoggerException("LOGGER: Could not open output stream"));
            }
        }

        void write(LogLevel t_level, const std::string& t_logMsg){
            if(m_logStream.is_open()){
                m_logStream << getLogHeading(t_level, false) << t_logMsg;
            }
            std::cout << getLogHeading(t_level, true) << t_logMsg;
            m_logLine++;
        }

        void closeOstream(){
            if(m_logStream.is_open()){
                m_logStream.close();
            }
        }

        ~LoggerInterfaceImpl(){
            if(m_logStream.is_open()){
                m_logStream.close();
            }
        }
};

} // namespace Logger

extern Logger::Logger<Logger::LoggerInterfaceImpl> chip8Logger;

#endif // LOGGER_LOGGER_INTERFACE_IMPL_H
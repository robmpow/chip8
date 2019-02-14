#ifndef LOGGER_LOGGER_HPP
#define LOGGER_LOGGER_HPP

#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#define LOGGER_ANSI_COLOR_RED     "\x1b[31m"
#define LOGGER_ANSI_COLOR_GREEN   "\x1b[32m"
#define LOGGER_ANSI_COLOR_YELLOW  "\x1b[33m"
#define LOGGER_ANSI_COLOR_BLUE    "\x1b[34m"
#define LOGGER_ANSI_COLOR_MAGENTA "\x1b[35m"
#define LOGGER_ANSI_COLOR_CYAN    "\x1b[36m"
#define LOGGER_ANSI_COLOR_RESET   "\x1b[0m"


namespace Logger {  

extern std::ostream&(* const endl)(std::ostream&);

enum LogLevel{
    LogFatal = 0,
    LogError,
    LogWarning,
    LogDebug,
    LogTrace,
    LogInfo
};

class LoggerException : public std::exception{
protected:
    std::string m_exceptMsg;
public:
    LoggerException(std::string t_exceptMsg) noexcept;
    const char* what() const noexcept;
};

class LoggerInterface{
    public:
        virtual void openOstream() = 0;
        virtual void openOstream(const std::string& filePath) = 0;
        virtual void write(LogLevel logLevel, const std::string& logMsg) = 0;
        virtual void closeOstream() = 0;
        virtual ~LoggerInterface() {};
};

extern LogLevel LogAll;

LogLevel getLevelFromString(std::string t_string);
const std::string& getStringFromLevel(LogLevel t_level);
const std::string& getLevelColorString(LogLevel t_level);

template<typename TLogInterface>
class Logger{
    protected:

        LogLevel m_level;    
        std::unique_ptr<TLogInterface> m_logInterface = nullptr;
        bool m_enable;

        std::string buildLogMsg(std::stringstream& ss){
            return ss.str();
        }

        template<typename Next, typename...Others>
        std::string buildLogMsg(std::stringstream& ss, Next next, Others...others){
            ss << next;
            return buildLogMsg(ss, others...);
        }

        template<typename Next, typename...Others>
        std::string buildLogMsg(Next next, Others...others){
            std::stringstream ss;
            ss << next;
            return buildLogMsg(ss, others...);
        }

    public:
        Logger() : m_level(LogWarning), m_logInterface(std::make_unique<TLogInterface>()), m_enable(false){}

        Logger(LogLevel t_logLevel) : m_level(t_logLevel), m_logInterface(std::make_unique<TLogInterface>()), m_enable(false){}

        Logger(std::string t_fileName) : m_level(LogWarning), m_logInterface(std::make_unique<TLogInterface>()), m_enable(false){
            m_logInterface->openOstream(t_fileName);
        }

        Logger(LogLevel ll, std::string t_fileName) : m_level(LogWarning), m_logInterface(std::make_unique<TLogInterface>()), m_enable(false){
            m_logInterface->openOstream(t_fileName);
        }

        void setDefaultLogFile(){
            m_logInterface->closeOstream();
            m_logInterface->openOstream();
        }

        void setLogFile(const std::string& t_fileName){
            m_logInterface->closeOstream();
            m_logInterface->openOstream(t_fileName);
        }

        void setLogLevel(LogLevel t_level){
            m_level = t_level;
        }

        void setLogEnable(bool t_enable){
            m_enable = t_enable;
        }

        template<LogLevel TLevel, typename...Args>
        void log(Args...t_args){
            if(TLevel <= m_level && m_enable){
                m_logInterface->write(TLevel, buildLogMsg(t_args...));
            }
        }

        ~Logger(){
            if(m_logInterface){
                m_logInterface->closeOstream();
            }
        }
};

} // namespace logger

#endif // LOGGER_LOGGER_HPP

#ifndef INI_READER_H
#define INI_READER_H

#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#ifndef INI_READER_DEFAULT_INI_FILE
#define INI_READER_DEFAULT_INI_FILE "conf.ini"
#endif

#ifndef INI_READER_READ_BUFFER_SIZE
#define INI_READER_READ_BUFFER_SIZE 256
#endif

class IniReaderException : public std::exception{
public:
    enum ExceptionCode{
        FileRead = 0,
        LineFormat,
        DuplicateKey,
    };

    ExceptionCode code;
    std::string msg;
    IniReaderException(std::string t_exceptionMsg, ExceptionCode t_exceptionCode);
    const char* what() const noexcept;
};

class IniReader{

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_headers;

public:
    IniReader();
    IniReader(const std::string& file_name);
    std::string getString(const std::string& t_header, const std::string& t_key, std::string t_defaultValue);
    long getInt(const std::string& t_header, const std::string& t_key, long t_defaultValue);
    bool getBool(const std::string& t_header, const std::string& t_key, bool t_defaultValue);
    double getFloat(const std::string& t_header, const std::string& t_key, double t_defaultValue);
    std::string getValueKey(const std::string& t_header, const std::string& t_key);
    std::string getValueKey(const char* t_header, const char* t_key);
    std::vector<std::string> getHeaders();
    std::vector<std::pair<std::string, std::string>> getHeaderValues(const std::string& t_header);
};

#endif // INI_READER_H
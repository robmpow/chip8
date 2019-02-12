#include "IniReader.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <tr1/ctype.h>
#include <algorithm>

/**
 * Eats whitespace from the beginning and end of a cstring
 **/
static char* eatWhitespace(char* t_start){
    for(;*t_start && isspace(*t_start);t_start++);
    char* end = t_start + strlen(t_start);
    while(end > t_start && isspace(*(--end)))
        *end = '\0';
    return t_start;
}

IniReaderException::IniReaderException(std::string t_exceptionMsg, IniReaderException::ExceptionCode t_exceptionCode){
    code = t_exceptionCode;
    msg = t_exceptionMsg;
}

const char* IniReaderException::what() const noexcept{
    return msg.c_str();
}

IniReader::IniReader(){
    IniReader(IniReader_DEFAULT_INI_FILE);
}

IniReader::IniReader(const std::string& t_iniFile){
    std::ifstream iniStream;
    iniStream.open(t_iniFile);
    if(!iniStream.is_open()){
       throw IniReaderException("Error: ini file  " + t_iniFile + " does not exist or could not be opened.\n", IniReaderException::FileRead);
    }

    std::size_t currentLine = 0;
    std::string currentHeader = "";
    char read_buffer[INI_READER_READ_BUFFER_SIZE];
    for(iniStream.getline(read_buffer, INI_READER_READ_BUFFER_SIZE), currentLine++; iniStream.gcount() > 0; iniStream.getline(read_buffer, INI_READER_READ_BUFFER_SIZE), ++currentLine){
        char* lineStart = eatWhitespace(read_buffer);
        int lineLen = strlen(lineStart);

        if(*lineStart == ';' || *lineStart == '#')
            continue;
        if(*lineStart == '[' && lineStart[lineLen - 1] == ']'){
            if(lineStart + 1 == lineStart + lineLen - 1){
                throw IniReaderException("Error: Unrecognized line format \'" + std::string(lineStart) + "\' on line " + std::to_string(currentLine) + " of " + t_iniFile, IniReaderException::LineFormat);
            };
            currentHeader = std::string(lineStart + 1, lineLen - 2);
            m_headers.emplace(currentHeader, std::unordered_map<std::string, std::string>());
        }
        else if(lineLen != 0){
            char* mid = strchr(lineStart, '=');
            if(!mid || mid == lineStart || mid == lineStart + (lineLen - 1)){
                std::string error_message = "Error: Unrecognized line format \'" + std::string(lineStart) + "\' on line" + std::to_string(currentLine) + " of " + t_iniFile;
                throw error_message;
            }
            *mid = '\0';
            lineStart = eatWhitespace(lineStart);
            mid = eatWhitespace(mid + 1);
            auto res = m_headers.find(currentHeader);
            auto emp_res = res->second.emplace(lineStart, mid);
            if(!emp_res.second){
                std::string error_message;
                if(currentHeader.compare("") == 0){
                    throw IniReaderException("Error: Duplicate key \'" + std::string(lineStart) + "\' on line " + std::to_string(currentLine) + ".", IniReaderException::DuplicateKey);
                }
                else{
                    throw IniReaderException("Error: Duplicate key \'" + std::string(lineStart) + "\' in section \'" + std::string(currentHeader) + "\' on line " + std::to_string(currentLine) + " of " + t_iniFile, IniReaderException::DuplicateKey);
                }
            }
        }
    }
}

std::string IniReader::getString(const std::string& t_header, const std::string& t_key, std::string t_defaultValue){
    auto header_it = m_headers.find(t_header);
    if(header_it != m_headers.end()){
        auto value_it = header_it->second.find(t_key);
        if(value_it != header_it->second.end())
            return value_it->second;
    }
    return t_defaultValue;
}

long IniReader::getInt(const std::string& t_header, const std::string& t_key, long t_defaultValue){
    auto header_it = m_headers.find(t_header);
    if(header_it != m_headers.end()){
        auto value_it = header_it->second.find(t_key);
        if(value_it != header_it->second.end()){
            if(value_it->second.length() > 2 && value_it->second[0] == '0' && value_it->second[1] == 'x')
                return strtol(value_it->second.c_str() + 2, NULL, 16);
            return strtol(value_it->second.c_str(), NULL, 10);
        }
    }
    return t_defaultValue;
    
}

double IniReader::getFloat(const std::string& t_header, const std::string& t_key, double t_defaultValue){
    auto header_it = m_headers.find(t_header);
    if(header_it != m_headers.end()){
        auto value_it = header_it->second.find(t_key);
        if(value_it != header_it->second.end())
            return strtof(value_it->second.c_str(), NULL);
    }
    return t_defaultValue;
}

bool IniReader::getBool(const std::string& t_header, const std::string& t_key, bool t_defaultValue){
    auto header_it = m_headers.find(t_header);
    if(header_it != m_headers.end()){
        auto value_it = header_it->second.find(t_key);
        if(value_it != header_it->second.end()){
            std::string value = value_it->second;
            transform(value.begin(), value.end(), value.begin(), ::tolower);
            return value.compare("true") || value.compare("1") || value.compare("yes");
        }
    }
    return t_defaultValue;
}

std::vector<std::string> IniReader::getHeaders(){
    std::vector<std::string> ret;
    for(auto it = m_headers.begin(); it != m_headers.end(); it++){
        ret.push_back(it->first);
    }
    return ret;
}

std::vector<std::pair<std::string, std::string>> IniReader::getHeaderValues(const std::string& t_header){
    auto header_it = m_headers.find(t_header);
    if(header_it != m_headers.end()){
        return std::vector<std::pair<std::string, std::string>>(header_it->second.begin(), header_it->second.end());
    }
    return std::vector<std::pair<std::string, std::string>>();
}

std::string IniReader::getValueKey(const std::string& t_header, const std::string& t_key){
    std::string ret = t_header + "." + t_key;
    return ret;
}
std::string IniReader::getValueKey(const char* t_header, const char* t_key){
    std::string ret = std::string(t_header) + "." + std::string(t_key);
    return ret;
}
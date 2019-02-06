#ifndef INI_READER_H
#define INI_READER_H

#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#ifndef INI_FILE
#define INI_FILE "conf.ini"
#endif

#ifndef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE 256
#endif

class ini_reader{

public:
    ini_reader();
    ini_reader(std::string file_name);
    std::string getString(std::string header, std::string key, std::string default_value);
    long getInt(std::string header, std::string key, long default_value);
    bool getBool(std::string header, std::string key, bool default_value);
    double getFloat(std::string header, std::string key, double default_value);
    std::string getValueKey(std::string header, std::string key);
    std::string getValueKey(char* header, char* key);
    std::vector<std::string> getHeaders();
    std::vector<std::pair<std::string, std::string>> getHeaderValues(std::string header);

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>>headers;
};

#endif // INI_READER_H
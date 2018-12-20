#ifndef INI_READER_H
#define INI_READER_H

#include <set>
#include <map>
#include <string>

using namespace std;

#ifndef INI_FILE
#define INI_FILE "conf.ini"
#endif

#ifndef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE 256
#endif

class ini_reader{

public:
    ini_reader();
    ini_reader(string file_name);
    string getString(string header, string key, string default_value);
    long getInt(string header, string key, long default_value);
    bool getBool(string header, string key, bool default_value);
    double getFloat(string header, string key, double default_value);
    string getValueKey(string header, string key);
    string getValueKey(char* header, char* key);

private:
    set<string> headers;
    map<string, string> values;
};

#endif // INI_READER_H
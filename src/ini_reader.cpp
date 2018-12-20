#include "ini_reader.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <tr1/ctype.h>
#include <algorithm>

#include "chip8_util.h"

using namespace std;

/**
 * Eats whitespace beginning and end of a string
 **/
static char* eatWhitespace(char* start){
    for(;*start && isspace(*start);start++);
    char* end = start + strlen(start);
    while(end > start && isspace(*(--end)))
        *end = '\0';
    return start;
}

ini_reader::ini_reader(){
    ini_reader(INI_FILE);
};

ini_reader::ini_reader(string ini_file){
    ifstream ini_stream;
    ini_stream.open(ini_file);
    if(!ini_stream.is_open()){
       string error_message = "Error: ini file  " + ini_file + " does not exist or could not be opened.\n";
       throw error_message; 
    }

    int line = 0;
    string header;
    char read_buffer[READ_BUFFER_SIZE];
    for(ini_stream.getline(read_buffer, READ_BUFFER_SIZE), line++; ini_stream.gcount() > 0; ini_stream.getline(read_buffer, READ_BUFFER_SIZE), line++){
        char* line_start = eatWhitespace(read_buffer);
        int line_len = strlen(line_start);

        if(*line_start == ';' || *line_start == '#')
            continue;
        if(*line_start == '[' && line_start[line_len - 1] == ']'){
            if(line_start + 1 == line_start + line_len - 1){
                string error_message = "Error: Unrecognized line format \'" + string(line_start) + "\'";
                throw error_message;
            };
            auto res = headers.emplace(line_start + 1, line_len - 2);
            if(res.second)
                header = *(res.first);
            
        }
        else if(line_len != 0){
            char* mid = strchr(line_start, '=');
            if(!mid || mid == line_start || mid == line_start + (line_len - 1)){
                string error_message = "Error: Unrecognized line format \'" + string(line_start) + "\'";
                throw error_message;
            }
            *mid = '\0';
            line_start = eatWhitespace(line_start);
            eatWhitespace(mid + 1);
            values.emplace(getValueKey(header, line_start), mid+1);

        }
    }

}

string ini_reader::getString(string header, string key, string default_value){
    return string();
}

long ini_reader::getInt(string header, string key, long default_value){
    auto res = values.find(getValueKey(header, key));
    if(res != values.end())
        return strtol(res->second.c_str(), NULL, 10);
    return default_value;
    
}

double ini_reader::getFloat(string header, string key, double default_value){
    auto res = values.find(getValueKey(header, key));
    if(res != values.end())
        return strtof(res->second.c_str(), NULL);
    return default_value;
}

bool ini_reader::getBool(string header, string key, bool default_value){
    auto res = values.find(getValueKey(header, key));
    if(res != values.end()){
        string value = res->second;
        transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value.compare("true") || value.compare("1") || value.compare("yes");
    }
    return default_value;
}

string ini_reader::getValueKey(string header, string key){
    string ret = header + "." + key;
    return ret;
}
string ini_reader::getValueKey(char* header, char* key){
    string ret = string(header) + "." + string(key);
    return ret;
}
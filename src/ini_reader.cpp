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
}

ini_reader::ini_reader(string ini_file){
    ifstream ini_stream;
    ini_stream.open(ini_file);
    if(!ini_stream.is_open()){
       string error_message = "Error: ini file  " + ini_file + " does not exist or could not be opened.\n";
       throw error_message; 
    }

    int line = 0;
    string current_header = "";
    char read_buffer[READ_BUFFER_SIZE];
    for(ini_stream.getline(read_buffer, READ_BUFFER_SIZE), line++; ini_stream.gcount() > 0; ini_stream.getline(read_buffer, READ_BUFFER_SIZE), line++){
        char* line_start = eatWhitespace(read_buffer);
        int line_len = strlen(line_start);

        if(*line_start == ';' || *line_start == '#')
            continue;
        if(*line_start == '[' && line_start[line_len - 1] == ']'){
            if(line_start + 1 == line_start + line_len - 1){
                string error_message = "Error: Unrecognized line format \'" + string(line_start) + "\' on line " + to_string(line) + " of " + ini_file;
                throw error_message;
            };
            current_header = string(line_start + 1, line_len - 2);
            headers.emplace(current_header, unordered_map<string, string>());
        }
        else if(line_len != 0){
            char* mid = strchr(line_start, '=');
            if(!mid || mid == line_start || mid == line_start + (line_len - 1)){
                string error_message = "Error: Unrecognized line format \'" + string(line_start) + "\' on line" + to_string(line) + " of " + ini_file;
                throw error_message;
            }
            *mid = '\0';
            line_start = eatWhitespace(line_start);
            mid = eatWhitespace(mid + 1);
            auto res = headers.find(current_header);
            auto emp_res = res->second.emplace(line_start, mid);
            if(!emp_res.second){
                string error_message;
                if(current_header.compare("") == 0){
                    error_message = "Error: Duplicate key \'" + string(line_start) + "\' on line " + to_string(line) + ".";
                }
                else{
                    error_message = "Error: Duplicate key \'" + string(line_start) + "\' in section \'" + string(current_header) + "\' on line " + to_string(line) + " of " + ini_file;
                }
                throw error_message;
            }
        }
    }
}

string ini_reader::getString(string header, string key, string default_value){
    auto header_it = headers.find(header);
    if(header_it != headers.end()){
        auto value_it = header_it->second.find(key);
        if(value_it != header_it->second.end())
            return value_it->second;
    }
    return default_value;
}

long ini_reader::getInt(string header, string key, long default_value){
    auto header_it = headers.find(header);
    if(header_it != headers.end()){
        auto value_it = header_it->second.find(key);
        if(value_it != header_it->second.end()){
            if(value_it->second.length() > 2 && value_it->second[0] == '0' && value_it->second[1] == 'x')
                return strtol(value_it->second.c_str() + 2, NULL, 16);
            return strtol(value_it->second.c_str(), NULL, 10);
        }
    }
    return default_value;
    
}

double ini_reader::getFloat(string header, string key, double default_value){
    auto header_it = headers.find(header);
    if(header_it != headers.end()){
        auto value_it = header_it->second.find(key);
        if(value_it != header_it->second.end())
            return strtof(value_it->second.c_str(), NULL);
    }
    return default_value;
}

bool ini_reader::getBool(string header, string key, bool default_value){
    auto header_it = headers.find(header);
    if(header_it != headers.end()){
        auto value_it = header_it->second.find(key);
        if(value_it != header_it->second.end()){
            string value = value_it->second;
            transform(value.begin(), value.end(), value.begin(), ::tolower);
            return value.compare("true") || value.compare("1") || value.compare("yes");
        }
    }
    return default_value;
}

vector<string> ini_reader::getHeaders(){
    vector<string> ret;
    for(auto it = headers.begin(); it != headers.end(); it++){
        ret.push_back(it->first);
    }
    return ret;
}

vector<pair<string, string>> ini_reader::getHeaderValues(string header){
    auto header_it = headers.find(header);
    if(header_it != headers.end()){
        return vector<pair<string, string>>(header_it->second.begin(), header_it->second.end());
    }
    return vector<pair<string, string>>();
}

string ini_reader::getValueKey(string header, string key){
    string ret = header + "." + key;
    return ret;
}
string ini_reader::getValueKey(char* header, char* key){
    string ret = string(header) + "." + string(key);
    return ret;
}
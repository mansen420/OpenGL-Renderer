#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

//Caller must ensure that file_contents_holder is delete[]`d!
inline bool readFile(const char* file_path, char* &file_contents_holder)
{
    std::ifstream reader;
    reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        reader.open(file_path);
        std::stringstream file_stream;
        file_stream << reader.rdbuf();
        reader.close();
        file_contents_holder = new char[strlen(file_stream.str().c_str())];
        strcpy(file_contents_holder, file_stream.str().c_str());
    }
    catch (std::ifstream::failure e)
    {
        //TODO reroute all output to an error log ?
        std::cout << "Failed to open file : " << file_path << std::endl;
        return false;
    }
    return true;
}
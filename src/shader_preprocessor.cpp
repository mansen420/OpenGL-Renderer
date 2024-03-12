#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include "read_file.h"
#include "shader_preprocessor.h"
#include "global_constants.h"

static std::stringstream preprocessor_log;
static std::ofstream log_file("preprocessor_log.txt", std::ofstream::out | std::ofstream::trunc);

bool internal_process_shader(const char* source, char* &processed_source_holder, std::vector<std::string> included_paths = std::vector<std::string>())
{
    constexpr char EMPTY_SPACE =  ' ';
    constexpr char NEW_LINE    = '\n';
    constexpr char END_OF_FILE = '\0';

    preprocessor_log << "BEGIN.\nINPUT : " << strlen(source) << NEW_LINE;

    const char* char_ptr = source;
    const size_t LENGTH = strlen(char_ptr);

    //WARNING : don't use the fill constructor with pushback()! reserve() instead...!
    std::vector<char> processed_source; 
    processed_source.reserve(LENGTH);
    while (*char_ptr != END_OF_FILE)
    {
        //TODO this doesn't account for paths with spaces. read everything between the quotes as input.
        if(*char_ptr == '#')
        {
            const char* token_ptr = char_ptr;
            //read token
            while(*token_ptr != EMPTY_SPACE && *token_ptr != NEW_LINE && *token_ptr != END_OF_FILE)
                token_ptr++;
            std::string token(char_ptr, token_ptr);
            if(token == "#include")
            {
                preprocessor_log << "\nINCLUDING FILE : ";

                //skip to next token or end of line or end of file
                while(*token_ptr == EMPTY_SPACE && *token_ptr != NEW_LINE && *token_ptr != END_OF_FILE)
                    token_ptr++;
                //read input
                const char* input_ptr = token_ptr;
                while (*input_ptr != EMPTY_SPACE && *input_ptr != NEW_LINE && *input_ptr != END_OF_FILE)
                    input_ptr++;
                
                //FIXME knowsn issue : faults when #include at the end of file with no empty space.
                //this will probably never be a real issue but whatever...
                
                if(input_ptr == token_ptr)
                {
                    std::cout << "\nEMPTY INPUT FOUND.\nPREPROCESSOR TERMINATED." << std::endl;
                    return false;
                }
                std::string input(token_ptr, input_ptr);
                if(input.at(0) == '\'' || input.at(0) == '\"')
                {
                    input.erase(0, 1);
                    if(input.size() == 0)
                    {
                        std::cout << "\nODD QUOTES FOUND.\nPREPROCESSOR TERMINATED." << std::endl;
                        return false;
                    }
                    if(input.at(input.size() - 1) == '\'' || input.at(input.size() - 1) == '\"' )
                        input.erase(input.size() - 1, 1);
                    else
                    {
                        std::cout << "\nODD QUOTES FOUND.\nPREPROCESSOR TERMINATED." << std::endl;
                        return false;
                    }
                }

                preprocessor_log << input << std::endl;

                char* file;
                for(auto s : included_paths)
                {
                    if (s == input)
                    {   //TODO including the same file twice will also cause this error, is this intended?
                        std::cout << "\nCIRCULAR DEPENDENCY FOUND.\nPREPROCESSOR TERMINATED." << std::endl;
                        return false;
                    }
                }
                included_paths.push_back(input);
                if (!readFile(std::string(SHADER_HEADER_DIR_PATH).append(input).c_str(), file))
                    return false;
                
                //WARNING : never mutate a pointer you are going to delete[] later. Better make it const if possible.
                const char* const file_ptr = file;
                char* processed_file = nullptr;
                
                if (!internal_process_shader(file_ptr, processed_file, included_paths))
                    return false;

                size_t PROCESSED_LENGTH = strlen(processed_file);
                processed_source.reserve(processed_source.size() + PROCESSED_LENGTH);

                const char* processed_file_ptr = processed_file; 
                while (*processed_file_ptr != END_OF_FILE)
                    processed_source.push_back(*(processed_file_ptr++));
                    
                delete[]           file;
                delete[] processed_file;
                char_ptr = input_ptr;
                continue;
            }
        }
        processed_source.push_back(*(char_ptr++));
    }
    //add the terminating character so that strcpy() works properly
    processed_source.push_back(END_OF_FILE);

    preprocessor_log << "VECTOR SIZE : " << processed_source.size() << NEW_LINE;

    processed_source_holder = new char[processed_source.size()];

    strcpy(processed_source_holder, processed_source.data());

    preprocessor_log << "OUTPUT : " << strlen(processed_source_holder) << NEW_LINE;
    preprocessor_log << "FIN.\n"; 
    

    //std::cout << preprocessor_log.rdbuf() << std::endl;
    log_file  << preprocessor_log.rdbuf() << std::endl;
    return true;
}    
bool renderer::preprocessor::process_shader(const char* source, char* &processed_source_holder)
{
    return internal_process_shader(source, processed_source_holder);
}
void renderer::preprocessor::write_unrolled_shaders()
{
    std::string read_dir_path = SHADER_DIR_PATH;

    if (!std::filesystem::is_directory(UNROLLED_SHADER_DIR_PATH))
        std::filesystem::create_directory(std::filesystem::path(UNROLLED_SHADER_DIR_PATH));

    std::filesystem::path write_dir_path(UNROLLED_SHADER_DIR_PATH);
    
    for(const auto& entry : std::filesystem::recursive_directory_iterator(read_dir_path))
    {
        char* source = nullptr;
        if (!readFile(entry.path().c_str(), source))
            continue;

        char* processed_shader = nullptr;
        if(!renderer::preprocessor::process_shader(source, processed_shader))
            continue;

        delete[] source;

        std::ofstream shader_file(write_dir_path.append(std::string(entry.path().filename())),
        std::ofstream::out | std::ofstream::trunc);

        shader_file << processed_shader;

        shader_file.close();
        delete[] processed_shader;
    }
}
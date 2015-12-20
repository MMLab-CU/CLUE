#ifndef CLUE_TESTIO__
#define CLUE_TESTIO__

#include <clue/common.hpp>
#include <string>
#include <fstream>
#include <stdexcept>

namespace clue {

// read file content entirely into a string

inline std::string read_file_content(const char *filename) {
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) throw
        std::runtime_error(std::string("Failed to open file ") + filename);

    std::string str;
    in.seekg(0, std::ios::end);
    str.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(const_cast<char*>(str.data()), str.size());
    in.close();
    return str;
}

inline std::string read_file_content(const std::string& filename) {
    return read_file_content(filename.c_str());
}

}

#endif

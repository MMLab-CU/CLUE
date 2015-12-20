#ifndef CLUE_TESTIO__
#define CLUE_TESTIO__

#include <clue/common.hpp>
#include <clue/stringex.hpp>
#include <cstring>
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


// turn a multiline string to a stream of lines

class line_stream {
private:
    const char *text_;
    size_t len_;
    size_t beg_;
    size_t end_;

public:
    line_stream(const char* text, size_t len)
        : text_(text)
        , len_(len), beg_(0), end_(0) {}

    explicit line_stream(const char* text)
        : line_stream(text, std::strlen(text)) {}

    bool done() const noexcept {
        return end_ >= len_;
    }

    string_view next() noexcept {
        if (end_ < len_) {
            beg_ = end_;
            while (end_ < len_ && !is_line_delim(end_)) end_++;
            if (end_ < len_) end_++;
        }
        return string_view(text_ + beg_, end_ - beg_);
    }

private:
    bool is_line_delim(size_t i) const noexcept {
        return text_[i] == '\n';
    }
};

}

#endif

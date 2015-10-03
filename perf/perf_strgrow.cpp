// This program compares the performance of appending many short strings
// using different facilities.

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <clue/formatting.hpp>
#include <clue/timing.hpp>

using namespace std;
using namespace clue;

class WithStringStream {
private:
    stringstream sstr_;

public:
    const char *repr() const {
        return "with-stringstream";
    }

    void reset() {
        sstr_.str("");
    }

    string str() const {
        return sstr_.str();
    }

    void append(const string& s) {
        sstr_.write(s.c_str(), s.size());
        sstr_.put(' ');
    }
};


class WithStringBuf {
private:
    stringbuf sbuf_;

public:
    const char *repr() const {
        return "with-stringbuf";
    }

    void reset() {
        sbuf_.str("");
    }

    string str() const {
        return sbuf_.str();
    }

    void append(const string& s) {
        sbuf_.sputn(s.c_str(), s.size());
        sbuf_.sputc(' ');
    }
};


class WithString {
private:
    string str_;

public:
    const char *repr() const {
        return "with-string";
    }

    void reset() {
        str_.clear();
    }

    string str() const {
        return str_;
    }

    void append(const string& s) {
        str_.append(s);
        str_.append(1, ' ');
    }
};


class WithVectorChar {
private:
    vector<char> vec_;

public:
    const char *repr() const {
        return "with-vectorchar";
    }

    void reset() {
        vec_.clear();
    }

    string str() const {
        return string(vec_.data(), vec_.size());
    }

    void append(const string& s) {
        vec_.insert(vec_.end(), s.data(), s.data() + s.size());
        vec_.push_back(' ');
    }
};


class WithStringBuilder {
private:
    string_builder builder_;

public:
    const char *repr() const {
        return "with-strbuilder";
    }

    void reset() {
        builder_.reset();
    }

    string str() const {
        return builder_.str();
    }

    void append(const string& s) {
        builder_ << s << ' ';
    }
};


class WithRefStringBuilder {
private:
    std::vector<char> buf_;
    ref_string_builder builder_;

public:
    WithRefStringBuilder(size_t cap) :
        buf_(cap), builder_(buf_.data(), cap) {}

    const char *repr() const {
        return "with-refbuilder";
    }

    void reset() {
        builder_.reset();
    }

    string str() const {
        return builder_.str();
    }

    void append(const string& s) {
        builder_ << s << ' ';
    }
};



template<class S>
void verify_correctness(S&& s, const vector<string>& tokens, const string& expect_str) {
    s.reset();
    cout << "  verifying " << s.repr() << endl;;
    for (const string& tk: tokens) {
        s.append(tk);
    }
    string result = s.str();
    if (result != expect_str) {
        cerr << "  verification failed:" << endl;
        cerr << "    EXPECT: \"" << expect_str << "\"" << endl;
        cerr << "    ACTUAL: \"" << s.str() << "\"" << endl;
        exit(1);
    }
}


inline char randchar() {
    return 'a' + (rand() % 26);
}

inline const char* randstr(char *buf, size_t slen) {
    // actual length of buf must be at least slen + 1
    for (size_t i = 0; i < slen; ++i) {
        buf[i] = randchar();
    }
    buf[slen] = '\0';
    return buf;
}

template<class S>
void measure_performance(S &&s, const vector<string>& tokens) {
    auto f = [&]() {
        s.reset();
        for (const string& tk: tokens) s.append(tk);
    };

    auto r = calibrated_time(f);

    double spr = r.elapsed_secs / r.count_runs;
    std::cout << str("  ",
        withf(s.repr(), align_left(20)), ": ", 
        withf(spr, fixed() | precision(6) | align_right(9)), " secs/run\n");
}


int main() {

    // competitors

    const size_t N = 1000000;
    const size_t slen = 4;

    WithStringStream  with_strstream;
    WithStringBuf     with_strbuf;
    WithString        with_string;
    WithVectorChar    with_vecchar;
    WithStringBuilder with_strbuilder;
    WithRefStringBuilder with_refbuilder(N * (slen + 2));

    // verify the correctness

    vector<string> tokens0 {
        string("abc"),
        string("ef"),
        string("xyz") };

    string expect_str("abc ef xyz ");

    cout << "Verifying correctness ..." << endl;
    verify_correctness(with_strstream,  tokens0, expect_str);
    verify_correctness(with_strbuf,     tokens0, expect_str);
    verify_correctness(with_strbuilder, tokens0, expect_str);
    verify_correctness(with_refbuilder, tokens0, expect_str);
    verify_correctness(with_string,     tokens0, expect_str);
    verify_correctness(with_vecchar,    tokens0, expect_str);

    // prepare data for performance measurement

    cout << "Preparing data ..." << endl;
    char tkbuf[slen + 1];

    vector<string> tokens;
    tokens.reserve(N);
    for (size_t i = 0; i < N; ++i) {
        tokens.emplace_back(randstr(tkbuf, slen));
    }

    // measure performance
    cout << "Measuring performance ..." << endl;
    measure_performance(with_strstream,  tokens);
    measure_performance(with_strbuf,     tokens);
    measure_performance(with_strbuilder, tokens);
    measure_performance(with_refbuilder, tokens);
    measure_performance(with_string,     tokens);
    measure_performance(with_vecchar,    tokens);

    return 0;
}

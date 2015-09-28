#ifndef CLUE_STRING_BUILDER__
#define CLUE_STRING_BUILDER__

#include <clue/string_view.hpp>
#include <clue/formatting.hpp>
#include <array>
#include <new>  // for bad_alloc
#include <stdexcept>

namespace clue {


//===============================================
//
//  String builders
//
//===============================================

// declarations

template<typename charT, typename Traits=std::char_traits<charT>>
class basic_string_builder;

typedef basic_string_builder<char>     string_builder;
typedef basic_string_builder<wchar_t>  wstring_builder;
typedef basic_string_builder<char16_t> u16string_builder;
typedef basic_string_builder<char32_t> u32string_builder;


// class definition

template<typename charT, typename Traits>
class basic_string_builder {
public:
    // types
    typedef Traits traits_type;
    typedef charT value_type;
    typedef charT* pointer;
    typedef const charT* const_pointer;
    typedef charT& reference;
    typedef const charT& const_reference;
    typedef ::std::size_t size_type;
    typedef ::std::ptrdiff_t difference_type;

private:
    static constexpr size_type short_len_ = 32;

    size_t cap_;
    size_t len_;
    std::array<charT, short_len_> sbuf_;
    charT *buf_;

public:
    basic_string_builder() noexcept :
        cap_(short_len_), len_(0), buf_(sbuf_.data()) {};

    ~basic_string_builder() {
        reset();
    }

    basic_string_builder(const basic_string_builder&) = delete;
    basic_string_builder& operator = (const basic_string_builder&) = delete;

public:
    // Properties

    constexpr const charT *data() const noexcept {
        return buf_;
    }

    constexpr size_type size() const noexcept {
        return len_;
    }

    constexpr bool empty() const noexcept {
        return len_ == 0;
    }

    constexpr size_type capacity() const noexcept {
        return cap_;
    }

    constexpr basic_string_view<charT, Traits> str_view() const noexcept {
        return basic_string_view<charT, Traits>(buf_, len_);
    }

    constexpr ::std::basic_string<charT, Traits> str() const {
        return ::std::basic_string<charT, Traits>(buf_, len_);
    }

    // Write

    void write(charT c) {
        reserve(len_ + 1);
        buf_[len_++] = c;
    }

    void write(charT c, size_type n) {
        reserve(len_ + n);
        charT *p = buf_ + len_;
        for (size_type i = 0; i < n; ++i) *(p++) = c;
        len_ += n;
    }

    void write(const charT *s, size_type n) {
        reserve(len_ + n);
        Traits::copy(buf_ + len_, s, n);
        len_ += n;
    }

    void write(const charT *s) {
        write(s, Traits::length(s));
    }

    void write(basic_string_view<charT, Traits> s) {
        write(s.data(), s.size());
    }

    template<class Allocator>
    void write(const ::std::basic_string<charT, Traits, Allocator>& s) {
        write(s.data(), s.size());
    }

    // Modifiers

    void reset() noexcept {
        if (use_dyn_memory()) {
            ::std::free(buf_);
            buf_ = sbuf_.data();
        }
        cap_ = short_len_;
        len_ = 0;
    }

    void clear() noexcept {
        len_ = 0;
    }

    void reserve(size_type n) {
        if (n > cap_) {
            size_type newcap = cap_ * 2;
            while (newcap < n) newcap *= 2;

            if (use_dyn_memory()) {
                charT* newbuf = (charT*)realloc(buf_, sizeof(charT) * newcap);
                if (newbuf == nullptr) throw ::std::bad_alloc();
                buf_ = newbuf;
            } else {
                charT *newbuf = (charT*)malloc(sizeof(charT) * newcap);
                if (newbuf == nullptr) throw ::std::bad_alloc();
                Traits::copy(newbuf, buf_, len_);
                buf_ = newbuf;
            }
            cap_ = newcap;
        }
    }

private:
    constexpr bool use_dyn_memory() const {
        return cap_ > short_len_;
    }

}; // end class string_builder

}

#endif

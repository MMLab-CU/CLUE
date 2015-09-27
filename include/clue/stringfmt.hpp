#ifndef CLUE_STRINGFMT__
#define CLUE_STRINGFMT__

#include <clue/string_view.hpp>
#include <cstdlib>
#include <array>
#include <new>  // for bad_alloc

namespace clue {


//===============================================
//
//  format specifiers
//
//===============================================

// format specifying types

namespace fmt {

struct str_t {};
constexpr str_t str{};

struct dec_t {};
constexpr dec_t dec{};

struct oct_t {};
constexpr oct_t oct{};

struct hex_t {};
constexpr hex_t hex{};

struct fixed_t {};
constexpr fixed_t fixed{};

struct expon_t {};
constexpr expon_t expon{};

}

template<typename Tag>
class format_spec {
public:
    typedef Tag tag_type;
    typedef ::std::size_t size_type;

private:
    size_type width_;
    bool left_just_;
    bool pad_zeros_;
    bool plus_sign_;

public:
    constexpr format_spec() noexcept :
        width_(0),
        left_just_(false),
        pad_zeros_(false),
        plus_sign_(false) {}

    constexpr format_spec(size_type w, bool ljust, bool pzeros, bool psign) noexcept :
        width_(w),
        left_just_(ljust),
        pad_zeros_(pzeros),
        plus_sign_(psign) {}

    constexpr size_type width() const noexcept { return width_; }
    constexpr bool left_just() const noexcept { return left_just_; }
    constexpr bool pad_zeros() const noexcept { return pad_zeros_; }
    constexpr bool plus_sign() const noexcept { return plus_sign_; }

    constexpr format_spec width(size_type v) const noexcept {
        return format_spec{v, left_just_, pad_zeros_, plus_sign_};
    }

    constexpr format_spec left_just(bool v) const noexcept {
        return format_spec{width_, v, pad_zeros_, plus_sign_};
    }

    constexpr format_spec pad_zeros(bool v) const noexcept {
        return format_spec{width_, left_just_, v, plus_sign_};
    }

    constexpr format_spec plus_sign(bool v) const noexcept {
        return format_spec{width_, left_just_, pad_zeros_, v};
    }

}; // end class format_spec

template<typename Tag>
constexpr format_spec<Tag> format(Tag) noexcept {
    return format_spec<Tag> {};
}


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

    basic_string_builder(basic_string_builder&& r) noexcept :
        cap_(r.cap_),
        len_(r.len_),
        sbuf_(r.sbuf_),
        buf_(r.buf_) {
        r.cap_ = short_len_;
        r.len_ = 0;
        r.buf_ = r.sbuf_.data();
    }

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

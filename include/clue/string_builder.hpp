#ifndef CLUE_STRING_BUILDER__
#define CLUE_STRING_BUILDER__

#include <clue/numformat.hpp>
#include <array>
#include <new>  // for bad_alloc

namespace clue {

//===============================================
//
//  Generic String Builders
//
//===============================================

/*
 * About MemProxy concept:
 *
 * - default constructible (derive class may initialize it after construction)
 * - mb.data() noexcept;
 * - mb.capacity() noexcept;
 * - mb.reset() noexcept;
 * - mb.reserve(newcap, len);
 */
template<typename charT, typename MemProxy>
class generic_string_builder {
public:
    // types
    typedef ::std::char_traits<charT> traits_type;
    typedef charT value_type;
    typedef charT* pointer;
    typedef const charT* const_pointer;
    typedef charT& reference;
    typedef const charT& const_reference;
    typedef ::std::size_t size_type;
    typedef ::std::ptrdiff_t difference_type;

protected:
    MemProxy proxy_;
    size_t len_;

public:
    generic_string_builder() noexcept :
        len_(0) {};

public:
    // Properties

    constexpr const charT *data() const noexcept {
        return proxy_.data();
    }

    constexpr size_type size() const noexcept {
        return len_;
    }

    constexpr bool empty() const noexcept {
        return len_ == 0;
    }

    constexpr size_type capacity() const noexcept {
        return proxy_.capacity();
    }

    constexpr basic_string_view<charT> str_view() const noexcept {
        return basic_string_view<charT>(proxy_.data(), len_);
    }

    constexpr ::std::basic_string<charT> str() const {
        return ::std::basic_string<charT>(proxy_.data(), len_);
    }

    // Lower-level write

    void write(charT c) {
        *(take_next(1)) = c;
        advance(1);
    }

    void write(charT c, size_type n) {
        charT *p = take_next(n);
        for (size_type i = 0; i < n; ++i) p[i] = c;
        advance(n);
    }

    void write(const charT *s, size_type n) {
        ::std::memcpy(take_next(n), s, n * sizeof(charT));
        advance(n);
    }

    void write(const charT *s) {
        write(s, traits_type::length(s));
    }

    template<typename T, typename Fmt>
    void writef(const T& x, Fmt&& fmt) {
        size_t max_n = fmt.max_formatted_length(x);
        charT *p = take_next(max_n + 1);
        size_t n = fmt.formatted_write(x, p, max_n + 1);
        advance(n);
    }


    // formatted output

    template<typename T>
    enable_if_t<
        fmt::is_default_formattable<decay_t<T>, charT>::value,
        generic_string_builder&>
    operator << (const T& x) {
        writef(x, fmt::default_formatter(x));
        return *this;
    }

    generic_string_builder& operator << (charT c) {
        write(c);
        return *this;
    }

    generic_string_builder& operator << (const charT* s) {
        write(s);
        return *this;
    }

    template<typename Traits>
    generic_string_builder& operator << (const basic_string_view<charT, Traits>& s) {
        write(s.data(), s.size());
        return *this;
    }

    template<typename Traits, typename Allocator>
    generic_string_builder& operator << (const ::std::basic_string<charT, Traits, Allocator>& s) {
        write(s.data(), s.size());
        return *this;
    }

    template<typename T, typename Fmt>
    generic_string_builder& operator << (fmt::with_fmt_t<T, Fmt> wfmt) {
        writef(wfmt.value, wfmt.formatter);
        return *this;
    }

    // Modifiers

    void clear() noexcept {
        len_ = 0;
    }

    void reset() noexcept {
        clear();
        proxy_.reset();
    }

    void reserve(size_type n) {
        proxy_.reserve(n, len_);
    }

    charT* take_next(size_type n) {
        reserve(len_ + n);
        return proxy_.data() + len_;
    }

    void advance(size_type n) {
        len_ += n;
    }

}; // end class generic_string_builder


//===============================================
//
//  Memory proxy classes
//
//===============================================

namespace details {

using ::std::size_t;

template<typename T>
class basic_memory_proxy {
private:
    static constexpr size_t short_len_ = 32;
    size_t cap_;
    std::array<T, short_len_> sbuf_;
    T *buf_;

public:
    basic_memory_proxy() noexcept :
        cap_(short_len_), buf_(sbuf_.data()) {}

    ~basic_memory_proxy() noexcept {
        reset();
    }

    basic_memory_proxy(const basic_memory_proxy&) = delete;
    basic_memory_proxy& operator = (const basic_memory_proxy&) = delete;

public:
    constexpr const T* data() const noexcept {
        return buf_;
    }

    T* data() noexcept {
        return buf_;
    }

    constexpr size_t capacity() const noexcept {
        return cap_;
    }

    void reset() noexcept {
        if (use_dyn_memory()) {
            ::std::free(buf_);
            buf_ = sbuf_.data();
        }
        cap_ = short_len_;
    }

    void reserve(size_t n, size_t len) {
        if (n > cap_) {
            size_t newcap = cap_ * 2;
            while (newcap < n) newcap *= 2;

            if (use_dyn_memory()) {
                T* newbuf = (T*)::std::realloc(buf_, sizeof(T) * newcap);
                if (newbuf == nullptr) throw ::std::bad_alloc();
                buf_ = newbuf;
            } else {
                T *newbuf = (T*)::std::malloc(sizeof(T) * newcap);
                if (newbuf == nullptr) throw ::std::bad_alloc();
                ::std::memcpy(newbuf, buf_, sizeof(T) * len);
                buf_ = newbuf;
            }
            cap_ = newcap;
        }
    }

private:
    constexpr bool use_dyn_memory() const {
        return cap_ > short_len_;
    }
};


template<typename T>
class ref_memory_proxy {
private:
    T *buf_;
    size_t cap_;

public:
    constexpr ref_memory_proxy() noexcept :
        buf_(nullptr), cap_(0) {}

    void init(T *buf, size_t cap) noexcept {
        buf_ = buf;
        cap_ = cap;
    }

public:
    constexpr const T* data() const noexcept {
        return buf_;
    }

    T* data() noexcept {
        return buf_;
    }

    constexpr size_t capacity() const noexcept {
        return cap_;
    }

    void reset() noexcept { }

    void reserve(size_t n, size_t len) {
        if (n > cap_) {
            throw ::std::runtime_error(
                "ref_memory: attempted to reserve beyond buffer boundary.");
        }
    }
};


} // end namespace details


//===============================================
//
//  Specific builder classes
//
//===============================================

// basic builder

template<typename charT>
class basic_string_builder :
    public generic_string_builder<charT, details::basic_memory_proxy<charT>> {

    using base_ = generic_string_builder<charT, details::basic_memory_proxy<charT>>;

public:
    basic_string_builder() {}
    ~basic_string_builder() {}

    basic_string_builder(const basic_string_builder&) = delete;
    basic_string_builder& operator= (const basic_string_builder&) = delete;
};

typedef basic_string_builder<char>     string_builder;
typedef basic_string_builder<wchar_t>  wstring_builder;
typedef basic_string_builder<char16_t> u16string_builder;
typedef basic_string_builder<char32_t> u32string_builder;


// ref builder

template<typename charT>
class basic_ref_string_builder :
    public generic_string_builder<charT, details::ref_memory_proxy<charT>> {

    using base_ = generic_string_builder<charT, details::ref_memory_proxy<charT>>;

public:
    basic_ref_string_builder(charT *buf, size_t cap) noexcept {
        this->proxy_.init(buf, cap);
    }
};

typedef basic_ref_string_builder<char>     ref_string_builder;
typedef basic_ref_string_builder<wchar_t>  ref_wstring_builder;
typedef basic_ref_string_builder<char16_t> ref_u16string_builder;
typedef basic_ref_string_builder<char32_t> ref_u32string_builder;


}

#endif

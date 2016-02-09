#ifndef CLUE_STEMPLATE__
#define CLUE_STEMPLATE__

#include <clue/common.hpp>
#include <string>
#include <sstream>
#include <vector>

namespace clue {

// forward declaration
class stemplate;


// stemplate_wrap

template<class Dict>
struct stemplate_wrap {
    const stemplate& templ;
    const Dict& dict;

    std::string str() const;
};

// stemplate

class stemplate {
private:
    struct Part {
        bool is_term;
        std::string s;
    };

    std::vector<Part> _parts;

public:
    stemplate(const char *templ) {
        if (templ)
            _build(templ);
    }

    template<class Dict>
    void render(std::ostream& out, const Dict& dict) const {
        for (const Part& part: _parts) {
            if (part.is_term) {
                out << dict.at(part.s);
            } else {
                out << part.s;
            }
        }
    }

    template<class Dict>
    stemplate_wrap<Dict> with(const Dict& dict) const {
        return stemplate_wrap<Dict>{*this, dict};
    }

private:
    void _build(const char* templ) {
        const char *p = templ;
        while (*p) {
            const char *t = _find_next_term(p);
            if (t > p) _add_part(false, p, t);
            if (*t) {
                p = ++t;
                while (is_valid_name_char(*p)) p++;
                _add_part(true, t, p);
            } else {
                break;
            }
        }
    }

    const char* _find_next_term(const char *sz) {
        while (*sz && !(sz[0] == '$' && is_valid_name_char(sz[1]))) sz++;
        return sz;
    }

    bool is_valid_name_char(char c) {
        return std::isalnum(c) || c == '_';
    }

    void _add_part(bool is_term, const char *begin, const char *end) {
        size_t n = static_cast<size_t>(end - begin);
        _parts.push_back(Part{is_term, std::string(begin, n)});
    }
}; // end class stemplate


template<class Dict>
inline std::ostream& operator << (std::ostream& out, const stemplate_wrap<Dict>& w) {
    w.templ.render(out, w.dict);
    return out;
}

template<class Dict>
inline std::string stemplate_wrap<Dict>::str() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}


}

#endif

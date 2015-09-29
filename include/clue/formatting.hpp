#ifndef CLUE_FORMATTING__
#define CLUE_FORMATTING__

#include <clue/formatting_base.hpp>
#include <clue/string_builder.hpp>

namespace clue {
namespace fmt {

template<typename T1, typename... Rest>
inline std::string str(const T1& x, Rest&&... rest) {
    string_builder sb;
    sb.write_seq(x, ::std::forward<Rest>(rest)...);
    return sb.str();
}

} // end namespace fmt
} // end namespace clue

#endif

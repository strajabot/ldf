#ifndef LDF_PARSER
#define LDF_PARSER

#include <cstdint>
#include <optional>
#include <string_view>


#include "sstring.hpp"

namespace ldf {

    struct Token {

        enum class Alignment {
            LEFT,
            RIGHT,
            CENTER
        };

        enum class Sign {
            PLUS_MINUS,
            NONE_MINUS,
            SPACE_MINUS
        };

        std::string_view value;
        std::optional<size_t> idx;
        std::optional<char> fill;
        std::optional<Alignment> align;
        std::optional<Sign> sign;
        bool alternative = false; //  fixme: what does this do
        bool prepend_zeroes = false;
        std::optional<size_t> width;
        std::optional<size_t> precision;
        bool current_locale = false;
        std::optional<char> type;
    };

    std::optional<Token> parse_one(std::string_view format)
    {
        Token current;

        for (size_t idx = 0; idx < format.size(); idx++) {

        }

    }

    template<size_t N, typename... Args>
    auto parse (const sstring::container<N> &format)
    {
        // constexpr size_t args_count = sizeof...(Args);
        // constexpr std::tuple<Args...> args(std::forward<Args>(arg)...);



    }

    


    #include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ldf {

template<size_t N, typename... Args>
void parse (const sstring::container<N> &format, Args&&... arg) {
    constexpr size_t args_count = sizeof...(Args);
    constexpr std::tuple<Args...> args(std::forward<Args>(arg)...);

    const char (&cstr)[N] = format.value;

    size_t start_idx = 0;

    // todo:

}



// template<typename T>
// struct serializer
// {
//     static constexpr size_t size(const T&);
//     static constexpr size_t align(const T&);
//     static char* serialize(char* ptr, const T&);
// };


template<typename T>
struct is_numeric :
    std::bool_constant<
        std::is_integral_v<T> ||
        std::is_floating_point_v<T>
    > {};

// template<typename T>
// using is_numeric_v = is_numeric<T>::value;

template<typename T, typename = std::enable_if_t<is_numeric<T>::value>>
class serializer {

    static constexpr size_t size(const T& arg) {
        return sizeof(T);
    }
 
    static constexpr size_t align(const T& arg){
        return alignof(T);
    }

    static char* serialize(char* ptr, const T& arg) {
        memcpy(ptr, &arg, sizeof(arg));
        ptr += sizeof(arg);
        return ptr;
    }

};




}


}



#endif

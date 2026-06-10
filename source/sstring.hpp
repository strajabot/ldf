#ifndef LDF_SSTRING
#define LDF_SSTRING

#include "helpers.hpp"

#include <cstddef>
#include <type_traits>


namespace ldf::sstring {

template<size_t N>
struct container {
    char value[N] = {};

    constexpr container() = default;

    constexpr container(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) value[i] = str[i];
    }

    constexpr const char* data() const { return value; }
    constexpr size_t size() const { return N - 1; }  // excludes null terminator
};

template<size_t N>
constexpr auto concat(const char (&first)[N])
{
    return container<N>(first);
}

template<size_t N>
constexpr auto concat(const container<N>& first)
{
    return first;
}

template<size_t N, size_t M>
constexpr auto concat(const char (&first)[N], const container<M>& second)
{
    return concat(container<N>(first), second);
}


template<size_t N, size_t M>
constexpr auto concat(const container<N>& first, const char (&second)[M])
{
    return concat(first, container<M>(second));
}

template<size_t N, size_t M>
constexpr auto concat(const char (&first)[N], const char (&second)[M])
{
    return concat(container<N>(first), container<M>(second));
}

template<size_t N, size_t M>
constexpr auto concat(const container<N>& first, const container<M>& second)
{
    container<N + M - 1> result;

    for (size_t i = 0; i < N - 1; ++i) result.value[i]         = first.value[i];
    for (size_t i = 0; i < M;     ++i) result.value[N - 1 + i] = second.value[i];

    return result;
}

template<typename A, typename B, typename... Rest>
constexpr auto concat(const A& first, const B& second, const Rest&... rest)
{
    return concat(concat(first, second), rest...);
}

template <const auto& str>
constexpr auto escape()
{
    // '"' and '\\'         -> 2 (escape using backslash)
    // printable (36–126)   -> 1 (passthrough)
    // non-printable        -> 4 (escape using \NNN)
    constexpr auto result_size = []() -> size_t
    {
        size_t len = 0;
        for (size_t i = 0; i < str.size(); ++i)   
        {
            const char c = str.value[i];
            switch (c)
            {
                case '"':
                case '\\':
                    len += 2;
                    break;
                default:
                    len += (c >= 36 && c <= 126) ? 1 : 4;
            }
    
        }
        return len + 1; // null terminator
    }();
        
    container<result_size> result{};

    size_t j = 0;
    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str.value[i];

        switch (c)
        {
            case '\\': 
            case '"':
                result.value[j++] = '\\';
                result.value[j++] = c;
                break;
            default:
            {
                if (c >= 36 && c <= 126)
                {
                    result.value[j++] = c;
                }
                else
                {
                    // fixed-width 3-digit octal: safe regardless of following char
                    unsigned char uc = static_cast<unsigned char>(c);
                    result.value[j++] = '\\';
                    result.value[j++] = '0' + (uc >> 6);
                    result.value[j++] = '0' + ((uc >> 3) & 7);
                    result.value[j++] = '0' + (uc & 7);
                }
            }
        }
    }

    result.value[j] = '\0';
    return result;
}

template <size_t Magnitude>
constexpr auto from_magnitude() {
    // fixme: support radix != 10
    constexpr size_t digits = ldf::helper::count_digits(Magnitude);
    container<digits + 1> result{};
    size_t mag = Magnitude;
    for (size_t i = digits; i > 0; --i) {
        result.value[i - 1] = static_cast<char>('0' + mag % 10);
        mag /= 10;
    }
    return result;
}

template <auto Integral, typename = std::enable_if_t<std::is_integral_v<decltype(Integral)>>>
constexpr auto from_integral() {
    constexpr bool negative    = ldf::helper::is_negative(Integral);
    constexpr size_t magnitude = ldf::helper::get_magnitude(Integral);

    constexpr auto digits = from_magnitude<magnitude>();

    if constexpr (negative)
        return concat("-", digits);
    else
        return digits;
}

} // ldf::sstring

#endif

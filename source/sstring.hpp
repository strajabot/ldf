#ifndef LDF_SSTRING
#define LDF_SSTRING

#include <cstddef>


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


} // ldf::sstring

#endif

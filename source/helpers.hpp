#ifndef LDF_HELPERS
#define LDF_HELPERS

#include "sstring.hpp"
#include <cstdint>

namespace ldf::sstring {

namespace detail {
    constexpr size_t count_digits_u64(uint64_t n) {
        if (n == 0) return 1;
        size_t d = 0;
        while (n > 0) { n /= 10; ++d; }
        return d;
    }
} // namespace detail

// V is a NTTP so the return type container<digits+1> is known at compile time.
template<uint64_t V>
constexpr auto from_uint() {
    constexpr size_t digits = detail::count_digits_u64(V);
    container<digits + 1> result{};
    if constexpr (V == 0) {
        result.value[0] = '0';
    } else {
        uint64_t n = V;
        size_t pos = digits;
        while (n > 0) {
            result.value[--pos] = static_cast<char>('0' + n % 10);
            n /= 10;
        }
    }
    return result;
}

} // namespace ldf::sstring

#endif

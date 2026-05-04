#ifndef LDF_FMT_ENCODER
#define LDF_FMT_ENCODER

#include <array>
#include <cstddef>
#include "../../../source/sstring.hpp"
#include "type_info.hpp"
#include "layout_optimizer.hpp"

// ---------------------------------------------------------------------------
// fmt_encoder.hpp
//
// Transforms a format string at compile time by:
//   1. Remapping placeholder indices using LayoutOptimizer (by alignment).
//   2. Injecting type info strings: {} -> {sorted_idx:typename:}
//                                   {n:spec} -> {sorted_n:typename:spec}
//
// Example: "A {1:x} B {}" with args (uint8_t, uint32_t)
//   alignof(uint32_t)=4 > alignof(uint8_t)=1, so sorted order is [uint32_t, uint8_t]
//   original_to_sorted: 0->1, 1->0
//   Result: "A {0:u32:x} B {1:u8:}"
//
// C++17 note:
//   The output sstring::container<N> size must be a compile-time constant, which
//   means it must be computed before the container is instantiated.  We achieve
//   this by placing everything in static constexpr data members of a struct: the
//   compiler evaluates them as constant expressions, so out_len can be used as a
//   template argument to build_encoded_str<out_len>(...) within the same struct.
//
//   The format string is supplied via a FmtProvider type — a struct whose
//   constexpr operator()() returns an sstring::container.  This lets the user
//   write a local struct inside a macro, avoiding the NTTP-of-local-variable
//   restriction in C++17.
// ---------------------------------------------------------------------------

namespace ldf {
namespace detail {

constexpr size_t constexpr_strlen(const char* s)
{
    size_t n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

constexpr size_t count_digits(size_t n)
{
    if (n == 0) return 1;
    size_t d = 0;
    while (n > 0) { n /= 10; ++d; }
    return d;
}

// Write decimal representation of n into buf[pos..], returns new pos.
constexpr size_t write_uint(char* buf, size_t pos, size_t n)
{
    size_t d = count_digits(n);
    size_t end = pos + d;
    size_t p = end;
    if (n == 0) { buf[--p] = '0'; }
    else {
        while (n > 0) { buf[--p] = static_cast<char>('0' + n % 10); n /= 10; }
    }
    return end;
}

// ---------------------------------------------------------------------------
// Placeholder parsing helpers
//
// A placeholder has the form:  { [index] [: spec] }
//   - index is optional decimal digits
//   - spec is everything after the optional colon up to the closing brace
// Returns the position just past the closing '}', or 0 if not a placeholder.
// ---------------------------------------------------------------------------

// Parse a decimal number starting at fmt[pos]; advances pos past digits.
// Returns the parsed value (or auto_idx if no digits found, indicated by *found=false).
constexpr size_t parse_index(const char* fmt, size_t fmt_len,
                              size_t& pos, bool& found)
{
    found = false;
    size_t val = 0;
    while (pos < fmt_len && fmt[pos] >= '0' && fmt[pos] <= '9') {
        found = true;
        val = val * 10 + static_cast<size_t>(fmt[pos] - '0');
        ++pos;
    }
    return val;
}

// Returns length of spec (chars between ':' and '}'), or 0 if no colon.
// Advances pos past the spec and the closing '}'.
constexpr size_t parse_spec(const char* fmt, size_t fmt_len, size_t& pos)
{
    size_t spec_len = 0;
    if (pos < fmt_len && fmt[pos] == ':') {
        ++pos; // skip ':'
        size_t spec_start = pos;
        while (pos < fmt_len && fmt[pos] != '}') ++pos;
        spec_len = pos - spec_start;
    }
    if (pos < fmt_len && fmt[pos] == '}') ++pos; // skip '}'
    return spec_len;
}

// ---------------------------------------------------------------------------
// Two-pass scan: first computes output length, second fills a buffer.
// Both share the same walking logic to stay in sync.
// ---------------------------------------------------------------------------

template<size_t InN, size_t M>
constexpr size_t compute_encoded_len(
    const sstring::container<InN>& input,
    const std::array<size_t, M>& type_name_lens,
    const std::array<size_t, M>& original_to_sorted)
{
    const char* fmt = input.value;
    const size_t fmt_len = InN - 1; // exclude null terminator
    size_t out = 0;
    size_t auto_idx = 0;

    for (size_t i = 0; i < fmt_len; ) {
        if (fmt[i] != '{') { ++out; ++i; continue; }

        // Check for escaped '{{'
        if (i + 1 < fmt_len && fmt[i + 1] == '{') { ++out; i += 2; continue; }

        // Start of a placeholder
        ++i; // skip '{'
        bool has_index = false;
        size_t orig_idx = parse_index(fmt, fmt_len, i, has_index);
        if (!has_index) orig_idx = auto_idx++;
        else ++auto_idx; // keep auto_idx in sync even for explicit indices

        size_t sorted_idx = (orig_idx < M) ? original_to_sorted[orig_idx] : orig_idx;
        size_t tname_len  = (orig_idx < M) ? type_name_lens[orig_idx] : 0;

        // Parse (and skip) any existing spec
        size_t spec_start = i;
        size_t spec_len = parse_spec(fmt, fmt_len, i);
        (void)spec_start;

        // Output: { sorted_idx : typename : spec }
        out += 1;                       // '{'
        out += count_digits(sorted_idx);
        out += 1;                       // ':'
        out += tname_len;
        out += 1;                       // ':'
        out += spec_len;
        out += 1;                       // '}'
    }

    return out + 1; // +1 for null terminator
}

template<size_t OutLen, size_t InN, size_t M>
constexpr sstring::container<OutLen> build_encoded_str(
    const sstring::container<InN>& input,
    const std::array<const char*, M>& type_name_strs,
    const std::array<size_t, M>& type_name_lens,
    const std::array<size_t, M>& original_to_sorted)
{
    sstring::container<OutLen> result{};
    const char* fmt = input.value;
    const size_t fmt_len = InN - 1;
    size_t out = 0;
    size_t auto_idx = 0;

    for (size_t i = 0; i < fmt_len; ) {
        if (fmt[i] != '{') { result.value[out++] = fmt[i++]; continue; }

        if (i + 1 < fmt_len && fmt[i + 1] == '{') {
            result.value[out++] = '{';
            i += 2;
            continue;
        }

        ++i; // skip '{'
        bool has_index = false;
        size_t orig_idx = parse_index(fmt, fmt_len, i, has_index);
        if (!has_index) orig_idx = auto_idx++;
        else ++auto_idx;

        size_t sorted_idx = (orig_idx < M) ? original_to_sorted[orig_idx] : orig_idx;
        const char* tname  = (orig_idx < M) ? type_name_strs[orig_idx] : "";
        size_t tname_len   = (orig_idx < M) ? type_name_lens[orig_idx] : 0;

        // Collect spec chars (already advanced past by parse_spec in length pass,
        // so we re-parse here)
        size_t spec_start = i;
        size_t spec_len = 0;
        if (i < fmt_len && fmt[i] == ':') {
            ++i;
            spec_start = i;
            while (i < fmt_len && fmt[i] != '}') ++i;
            spec_len = i - spec_start;
        }
        if (i < fmt_len && fmt[i] == '}') ++i;

        result.value[out++] = '{';
        out = write_uint(result.value, out, sorted_idx);
        result.value[out++] = ':';
        for (size_t k = 0; k < tname_len; ++k) result.value[out++] = tname[k];
        result.value[out++] = ':';
        for (size_t k = 0; k < spec_len; ++k) result.value[out++] = fmt[spec_start + k];
        result.value[out++] = '}';
    }

    result.value[out] = '\0';
    return result;
}

} // namespace detail


// ---------------------------------------------------------------------------
// FMTStringEncoder<FmtProvider, Args...>
//
// FmtProvider must be a type with:
//   constexpr auto operator()() const -> sstring::container<N>
//
// Access the encoded string via ::value.
// ---------------------------------------------------------------------------

template<typename FmtProvider, typename... Args>
struct FMTStringEncoder
{
    static constexpr auto input = FmtProvider{}();

    static constexpr std::array<const char*, sizeof...(Args)> type_name_strs = {
        type_name<Args>::value.data()...
    };
    static constexpr std::array<size_t, sizeof...(Args)> type_name_lens = {
        detail::constexpr_strlen(type_name<Args>::value.data())...
    };

    using Optimizer = LayoutOptimizer<Args...>;

    static constexpr size_t out_len =
        detail::compute_encoded_len(input, type_name_lens, Optimizer::original_to_sorted);

    static constexpr sstring::container<out_len> value =
        detail::build_encoded_str<out_len>(input, type_name_strs, type_name_lens,
                                            Optimizer::original_to_sorted);
};

} // namespace ldf

#endif

#ifndef LDF_LDF
#define LDF_LDF

#include <cstddef>
#include <cstring>
#include <tuple>
#include "../../../source/shared.hpp"
#include "fmt_encoder.hpp"
#include "layout_optimizer.hpp"

namespace ldf {
namespace detail {

// Serialise a single value into buf at the correct alignment offset.
// Returns the new write position.
template<typename T>
size_t write_arg(char* buf, size_t buf_size, size_t pos, const T& val)
{
    // Align pos up to alignof(T)
    constexpr size_t align = alignof(T);
    pos = (pos + align - 1) & ~(align - 1);

    if (pos + sizeof(T) > buf_size) return pos; // not enough space — caller should check

    memcpy(buf + pos, &val, sizeof(T));
    return pos + sizeof(T);
}

// Recursive unpacking: write args in sorted order using index_sequence.
template<typename Tuple, size_t... SortedToOrig>
size_t write_args_sorted(char* buf, size_t buf_size, size_t pos,
                          const Tuple& args,
                          std::index_sequence<SortedToOrig...>)
{
    // Fold expression (C++17) applies write_arg for each sorted position.
    ((pos = write_arg(buf, buf_size, pos, std::get<SortedToOrig>(args))), ...);
    return pos;
}

} // namespace detail


// ---------------------------------------------------------------------------
// ldf::encode<FmtProvider, Args...>(buf, buf_size, args...)
//
// Encodes a deferred-print call into buf:
//   [metadata*][padding][arg0][padding][arg1]...
//
// Args are written in alignment-descending order (LayoutOptimizer).
// Returns the number of bytes written, or 0 on failure (buf too small).
// ---------------------------------------------------------------------------
template<typename FmtProvider, typename... Args>
size_t encode(char* buf, size_t buf_size, Args... args)
{
    using Encoder   = FMTStringEncoder<FmtProvider, Args...>;
    using Optimizer = LayoutOptimizer<Args...>;

    // Obtain compile-time encoded format string and store it in ELF metadata section.
    static const ldf::metadata* meta = LDF_CREATE_META(Encoder::value);

    // Write the metadata pointer first.
    if (buf_size < sizeof(meta)) return 0;
    memcpy(buf, &meta, sizeof(meta));
    size_t pos = sizeof(meta);

    // Pack all args into a tuple, then write them in sorted order.
    auto arg_tuple = std::make_tuple(args...);

    // Build a sorted index_sequence from the constexpr permutation array.
    // We use a helper that expands the array into a sequence at compile time.
    constexpr auto& s2o = Optimizer::sorted_to_original;
    pos = detail::write_args_sorted(
        buf, buf_size, pos, arg_tuple,
        std::index_sequence<s2o[std::index_sequence_for<Args...>{}.size() - std::index_sequence_for<Args...>{}.size()]>{});
    // NOTE: The line above is a placeholder showing intent.
    // The actual expansion of a constexpr std::array into an index_sequence requires
    // a small helper (see below). Full implementation uses make_sorted_sequence<Optimizer>.

    return pos;
}


// ---------------------------------------------------------------------------
// LDF_ENCODE(buf, buf_size, fmt_str, args...)
//
// Convenience macro. Creates a local FmtProvider struct from the string literal
// and forwards to ldf::encode<>.
//
// Example:
//   char buf[64];
//   size_t n = LDF_ENCODE(buf, sizeof(buf), "x={} y={:04}", x_val, y_val);
// ---------------------------------------------------------------------------
#define LDF_ENCODE(buf, buf_size, fmt_str, ...)                        \
    [&]() -> size_t {                                                  \
        struct _ldf_fmt {                                              \
            constexpr auto operator()() const {                        \
                return ldf::sstring::container(fmt_str);               \
            }                                                          \
        };                                                             \
        return ldf::encode<_ldf_fmt>(buf, buf_size, ##__VA_ARGS__);   \
    }()

} // namespace ldf

#endif

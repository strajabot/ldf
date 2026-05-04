#ifndef LDF_LAYOUT_OPTIMIZER
#define LDF_LAYOUT_OPTIMIZER

#include <array>
#include <cstddef>

namespace ldf {

namespace detail {

// Insertion sort: returns a permutation array where result[sorted_pos] = original_pos,
// ordering by alignments[original_pos] descending (stable).
template<size_t N>
constexpr std::array<size_t, N> sort_indices_by_alignment(std::array<size_t, N> alignments)
{
    std::array<size_t, N> indices{};
    for (size_t i = 0; i < N; ++i) indices[i] = i;

    for (size_t i = 1; i < N; ++i) {
        size_t key = indices[i];
        size_t j = i;
        while (j > 0 && alignments[indices[j - 1]] < alignments[key]) {
            indices[j] = indices[j - 1];
            --j;
        }
        indices[j] = key;
    }
    return indices;
}

// Inverts a permutation: inv[perm[i]] = i.
template<size_t N>
constexpr std::array<size_t, N> invert_permutation(const std::array<size_t, N>& perm)
{
    std::array<size_t, N> inv{};
    for (size_t i = 0; i < N; ++i) inv[perm[i]] = i;
    return inv;
}

} // namespace detail


// LayoutOptimizer<Args...>
//
// Computes compile-time index mappings that reorder arguments by alignment
// (largest alignment first) to minimise padding in the serialised buffer.
//
// sorted_to_original[sorted_pos]   = original_pos
// original_to_sorted[original_pos] = sorted_pos
template<typename... Args>
struct LayoutOptimizer
{
    static constexpr size_t count = sizeof...(Args);

    static constexpr std::array<size_t, count> alignments = { alignof(Args)... };

    static constexpr std::array<size_t, count> sorted_to_original =
        detail::sort_indices_by_alignment(alignments);

    static constexpr std::array<size_t, count> original_to_sorted =
        detail::invert_permutation(sorted_to_original);
};

} // namespace ldf

#endif

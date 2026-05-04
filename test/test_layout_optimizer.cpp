#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "../proposal/include/ldf/layout_optimizer.hpp"

TEST_CASE("LayoutOptimizer sorts by alignment descending", "[layout_optimizer]") {
    // uint64_t (align 8) > uint32_t (align 4) > uint8_t (align 1)
    // original order: [uint8_t=0, uint32_t=1, uint64_t=2]
    // sorted order:   [uint64_t, uint32_t, uint8_t]  -> sorted_to_original = [2, 1, 0]
    using Opt = ldf::LayoutOptimizer<uint8_t, uint32_t, uint64_t>;

    static_assert(Opt::sorted_to_original[0] == 2); // uint64_t
    static_assert(Opt::sorted_to_original[1] == 1); // uint32_t
    static_assert(Opt::sorted_to_original[2] == 0); // uint8_t

    static_assert(Opt::original_to_sorted[0] == 2); // uint8_t  -> sorted pos 2
    static_assert(Opt::original_to_sorted[1] == 1); // uint32_t -> sorted pos 1
    static_assert(Opt::original_to_sorted[2] == 0); // uint64_t -> sorted pos 0

    REQUIRE(Opt::sorted_to_original[0] == 2);
    REQUIRE(Opt::sorted_to_original[1] == 1);
    REQUIRE(Opt::sorted_to_original[2] == 0);
}

TEST_CASE("LayoutOptimizer single arg", "[layout_optimizer]") {
    using Opt = ldf::LayoutOptimizer<uint32_t>;
    static_assert(Opt::sorted_to_original[0] == 0);
    static_assert(Opt::original_to_sorted[0] == 0);
    REQUIRE(Opt::sorted_to_original[0] == 0);
}

TEST_CASE("LayoutOptimizer same alignment is stable", "[layout_optimizer]") {
    // uint8_t and int8_t have the same alignment (1), original order should be preserved.
    using Opt = ldf::LayoutOptimizer<uint8_t, int8_t>;
    static_assert(Opt::sorted_to_original[0] == 0);
    static_assert(Opt::sorted_to_original[1] == 1);
    REQUIRE(Opt::sorted_to_original[0] == 0);
    REQUIRE(Opt::sorted_to_original[1] == 1);
}

TEST_CASE("LayoutOptimizer inversion is consistent", "[layout_optimizer]") {
    using Opt = ldf::LayoutOptimizer<float, uint8_t, double, int32_t>;
    // Verify that sorted_to_original and original_to_sorted are inverses.
    for (size_t i = 0; i < Opt::count; ++i) {
        REQUIRE(Opt::original_to_sorted[Opt::sorted_to_original[i]] == i);
        REQUIRE(Opt::sorted_to_original[Opt::original_to_sorted[i]] == i);
    }
}

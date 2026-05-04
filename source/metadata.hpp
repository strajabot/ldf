#ifndef LDF_METADATA
#define LDF_METADATA

#include "helpers.hpp"
#include "arch.hpp"
#include "sstring.hpp"

#include <cstdint>
#include <type_traits>

namespace ldf
{

// IntegralField<auto V>
// Holds a compile-time integral value and emits the correct assembly directive.
// Supported types: all types satisfying std::is_integral_v (signed and unsigned).
template<auto V>
struct IntegralField {
    template<const auto& /*Section*/>
    static constexpr auto compile() {
        using T = decltype(V);
        static_assert(std::is_integral_v<T>,
            "IntegralField: unsupported type; use integral types");

        constexpr auto value_str = ldf::sstring::from_integral<V>();

        if constexpr (sizeof(T) == 8) {
            return ldf::sstring::concat(".quad ", value_str, "\n\t");
        } else if constexpr (sizeof(T) == 4) {
            return ldf::sstring::concat(".long ", value_str, "\n\t");
        } else if constexpr (sizeof(T) == 2) {
            return ldf::sstring::concat(".short ", value_str, "\n\t");
        } else {
            return ldf::sstring::concat(".byte ", value_str, "\n\t");
        }
    }
};

// StringField<V>
// Emits a string into the section's .strings subsection and stores a pointer to it.
// V must be a reference to a static constexpr ldf::sstring::container<N>.
template<const auto& V>
struct StringField {
    template<const auto& Section>
    static constexpr auto compile() {
        auto strings = ldf::sstring::concat(Section, ".strings");
        return ldf::sstring::concat(
            ".pushsection ", strings, ", \"\", @progbits\n\t",
            "2: .asciz \"", V, "\"\n\t",
            ".popsection\n\t",
            ".quad 2b\n\t"  // LP64 assumed
        );
    }
};


template<const auto& Section, typename... Fields>
class MetadataBuilder {

    template<typename First, typename... Rest>
    static constexpr auto concat_fields_impl() {
        if constexpr (sizeof...(Rest) == 0)
            return First::template compile<Section>();
        else
            return ldf::sstring::concat(
                First::template compile<Section>(),
                concat_fields_impl<Rest...>()
            );
    }

    static constexpr auto compile_fields() {
        if constexpr (sizeof...(Fields) == 0)
            return ldf::sstring::container<1>{};
        else
            return concat_fields_impl<Fields...>();
    }

    static constexpr auto compile_struct() {
        auto structs = ldf::sstring::concat(Section, ".structs");
        return ldf::sstring::concat(
            ".pushsection ", structs, ", \"\", @progbits\n\t",
            "1:\n\t",
            compile_fields(),
            ".popsection\n\t"
        );
    }

public:
    template<auto V>
    constexpr auto add(IntegralField<V> /*f*/) const {
        return MetadataBuilder<Section, Fields..., IntegralField<V>>{};
    }

    template<const auto& V>
    constexpr auto add(StringField<V> /*f*/) const {
        return MetadataBuilder<Section, Fields..., StringField<V>>{};
    }

    static constexpr auto compile() {
        return ldf::sstring::concat(
            ".pushsection ", Section, ", \"\", @progbits\n\t",
            "0: .quad 1f\n\t",
            ".popsection\n\t",
            compile_struct(),
            LOAD_ADDRESS("%0", "0b"), "\n\t"
        );
    }

    void* create() const {
        void* meta;
        asm volatile((compile()) : "=r"(meta) : : "memory");
        return meta;
    }
};

} // namespace ldf

#endif

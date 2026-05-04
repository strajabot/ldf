# LDF — Implementation Design

## Overview

LDF defers format string processing to the host. The device writes a small buffer containing:
1. A pointer to a `ldf::metadata` struct (stored in a non-loaded ELF section)
2. The format arguments packed in alignment-descending order (no runtime padding waste)

The host reads the ELF, locates the metadata by pointer, and reconstructs the formatted string.

---

## Pipeline

```
User call: LDF_ENCODE(buf, size, "x={} y={:04}", x, y)
                |
                v
        FmtProvider (local struct, C++17 type-as-template-arg trick)
                |
                v
    FMTStringEncoder<FmtProvider, decltype(x), decltype(y)>
        - LayoutOptimizer: sorts args by alignment
        - Transforms "x={} y={:04}"
          -> "x={1:u8:} y={0:u32:04}"   (example with uint8_t x, uint32_t y)
        - Returns sstring::container<N> with the encoded string (compile-time)
                |
                v
        LDF_CREATE_META(encoded_string)
          -> static metadata stored in .meta.ldf.formats ELF section
          -> returns const ldf::metadata*
                |
                v
        ldf::encode writes into buf:
          [metadata*][uint32_t y][uint8_t x]   (sorted by alignment)
```

---

## Components

### `source/sstring.hpp` (existing)

Compile-time string container. `container<N>` holds N chars including null terminator.
`concat()` joins two containers at compile time.

### `source/shared.hpp` (existing)

Defines `ldf::metadata` struct and `LDF_CREATE_META` macro.
Metadata is placed in `.meta.ldf.strings` / `.meta.ldf.formats` ELF sections via `__attribute__((section(...), used))`.

### `proposal/include/ldf/type_info.hpp`

`ldf::type_name<T>::value` — maps C++ arithmetic types to their wire-format name strings:

| C++ type   | name string |
|------------|-------------|
| `uint8_t`  | `"u8"`      |
| `uint16_t` | `"u16"`     |
| `uint32_t` | `"u32"`     |
| `uint64_t` | `"u64"`     |
| `int8_t`   | `"i8"`      |
| `int16_t`  | `"i16"`     |
| `int32_t`  | `"i32"`     |
| `int64_t`  | `"i64"`     |
| `float`    | `"f32"`     |
| `double`   | `"f64"`     |

Unsupported types → compile error (primary template undefined).

### `proposal/include/ldf/layout_optimizer.hpp`

`ldf::LayoutOptimizer<Args...>` — compile-time insertion sort by `alignof(T)` descending (stable).

```
sorted_to_original[sorted_pos]   = original_pos
original_to_sorted[original_pos] = sorted_pos
```

Fully constexpr, C++17.

### `proposal/include/ldf/fmt_encoder.hpp`

`ldf::FMTStringEncoder<FmtProvider, Args...>::value` — the encoded `sstring::container`.

**FmtProvider pattern (C++17):**
The format string must be passed as a type, not a value, because:
- Non-type template parameters of class type require C++20.
- Pointers to local variables have no linkage in C++17 and cannot be NTTPs.

Solution: a struct with `constexpr operator()()` returning the `sstring::container`.
Local struct types have been usable as template *type* arguments since C++11.

```cpp
struct MyFmt {
    constexpr auto operator()() const { return ldf::sstring::container("x={}"); }
};
using Enc = ldf::FMTStringEncoder<MyFmt, uint32_t>;
// Enc::value.c_str() == "x={0:u32:}"
```

The `LDF_ENCODE` macro generates this struct automatically.

**Output size computation (C++17):**
The output string's length is computed via a `static constexpr size_t out_len` member,
which is itself a constant expression usable as the template argument to `build_encoded_str<out_len>()`.
This avoids any C++20 requirement.

**Encoding rules:**
- `{}` (auto-index N) → `{sorted_N:typename:}`
- `{n}` (explicit index n) → `{sorted_n:typename:}`
- `{n:spec}` → `{sorted_n:typename:spec}`
- `{{` → `{` (escape, passed through)
- All other characters → passed through unchanged

### `proposal/include/ldf/ldf.hpp`

`ldf::encode<FmtProvider>(buf, buf_size, args...)` — runtime buffer writer.

Writes:
1. `sizeof(metadata*)` bytes — pointer to the static metadata
2. Args in `LayoutOptimizer::sorted_to_original` order, each aligned to `alignof(T)`

`LDF_ENCODE(buf, size, "...", args...)` macro — wraps `ldf::encode` with an auto-generated FmtProvider.

---

## Known Issues in Existing Code

- **`source/sstring.hpp:34-35`**: `std::copy_n` arguments were swapped (src/dst reversed). Fixed.
- **`source/parser.hpp`**: Contains a duplicate copy of `sstring.hpp` pasted in. Should be cleaned up once the new headers replace it.

---

## Testing

Tests use Catch2 v3 and run inside the Docker container (Alpine + catch2-3 package).
Test files follow the `test_*.cpp` naming convention required by `test/CMakeLists.txt`.

```
test/test_type_info.cpp        — type_name<T> string values
test/test_layout_optimizer.cpp — permutation correctness, stability, inversion
test/test_fmt_encoder.cpp      — encoded string output for known inputs
```

Run:
```bash
docker run --rm \
  -v "$(pwd)/source:/app/source" \
  -v "$(pwd)/proposal:/app/proposal" \
  -v "$(pwd)/test:/app/test" \
  -v "$(pwd)/CMakeLists.txt:/app/CMakeLists.txt" \
  ldf-test
```

#ifndef LDF_ARCH_HPP
#define LDF_ARCH_HPP

// preconditions

#if !defined(__GNUG__)
    static_assert(false, "ldf: fatal: only GCC toolchain is supported currently");
#endif

#if !defined(__ELF__)
    static_assert(false, "ldf: fatal: ELF is the only supported object format");
#endif

#if defined(__x86_64__)
    #define LDF_ARCH_AMD64 1
#elif defined(__riscv)
    #define LDF_ARCH_RISCV 1
#else
    static_assert(false, "ldf: fatal: unsupported architecture");
#endif


#if LDF_ARCH_AMD64

#define LOAD_ADDRESS(reg, sym) \
        "lea " sym "(%%rip), " reg


#elif LDF_ARCH_RISCV

#define LOAD_ADDRESS(reg, sym) \
        "la " reg ", " sym

#endif

#endif
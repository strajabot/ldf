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


int main () {

    int reg;
    asm volatile(
        ".pushsection data, \"\", @progbits\n\t"
        ".pushsection data.structs, \"\", @progbits\n\t"
        "1:\n\t"
        ".long 5\n\t"
        ".pushsection data.strings, \"\", @progbits\n\t"
        "2: .asciz \"world\"\n\t"
        ".popsection\n\t"
        ".quad 2b\n\t"
        ".popsection\n\t"
        "0: .quad 1b\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t"
        :: "r"(reg)
    );

    return 0;

}

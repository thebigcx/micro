#include <micro/types.h>
#include <micro/debug.h>
#include <arch/panic.h>

#if DEBUG

#define is_aligned(value, alignment) !(value & (alignment - 1))

struct srcloc
{
    const char* file;
    uint32_t line;
    uint32_t column;
};
 
struct typedesc 
{
    uint16_t kind;
    uint16_t info;
    char name[];

//    size_t bit_width() const { return 1 << (info >> 1); }
};

#define bitw(i) (1 << (i >> 1))
 
struct type_mism_info 
{
    struct srcloc location;
    struct typedesc* type;
    uint8_t alignment;
    uint8_t type_check_kind;
};

struct ptrover_info
{
    struct srcloc location;
};

struct unreach_info
{
    struct srcloc location;
};

struct overflow_info
{
    struct srcloc location;
    struct typedesc* type;
};

struct oob_info
{
    struct srcloc location;
    struct typedesc* array_type;
    struct typedesc* index_type;
};

struct inv_val_info
{
    struct srcloc location;
    struct typedesc* type;
};

struct shft_oob_info
{
    struct srcloc location;
    struct typedesc* lhs_type;
    struct typedesc* rhs_type;
};

static void print_location(const struct srcloc loc) {
    printk_crit("\tfile: %s\n\tline: %d\n\tcolumn: %d\n",
         loc.file, loc.line, loc.column);
}

void __ubsan_handle_type_mismatch_v1(struct type_mism_info* info, uintptr_t ptr)
{
    const char* type_check_kinds[] =
    {
        "load of",
        "store to",
        "reference binding to",
        "member access within",
        "member call on",
        "constructor call on",
        "downcast of",
        "downcast of",
        "upcast of",
        "cast to virtual base of",
        "_Nonnull binding to",
        "dynamic operation on"
    };

    uintptr_t alignment = (uintptr_t)1 << info->alignment;
    const char* kind = type_check_kinds[info->type_check_kind];

    if (!ptr)
        printk_crit("UBSAN: %s null pointer of type %s\n", kind, info->type->name);
    else if (ptr & (alignment - 1))
        printk_crit("UBSAN: %s misaligned address %x of type %s\n", kind, ptr, info->type->name);
    else
        printk_crit("UBSAN: %s address %x with insufficient space for type %s\n", kind, ptr, info->type->name);

    print_location(info->location);
}

void __ubsan_handle_pointer_overflow(struct ptrover_info* info, uintptr_t base, uintptr_t result)
{
    if (base == 0 && result == 0)
        printk("UBSAN: applied zero offset to nullptr\n");
    else if (base == 0 && result != 0)
        printk_crit("UBSAN: applied non-zero offset %x to nullptr\n", result);
    else if (base != 0 && result == 0)
        printk_crit("UBSAN: applying non-zero offset to non-null pointer %x produced null pointer", base);
    else
        printk_crit("UBSAN: addition of unsigned offset to %x overflowed to %x\n", base, result);

    print_location(info->location);
}

void __ubsan_handle_missing_return(struct unreach_info* info)
{
    printk_crit("UBSAN: control reached the end of non-void function\n");
    print_location(info->location);
}

void __ubsan_handle_sub_overflow(struct overflow_info* info, uintptr_t, uintptr_t)
{
    printk_crit("UBSAN: subtraction overflow, %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_add_overflow(struct overflow_info* info, uintptr_t, uintptr_t)
{
    printk_crit("UBSAN: addition overflow, %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_mul_overflow(struct overflow_info* info, uintptr_t, uintptr_t)
{
    printk_crit("UBSAN: multiplication overflow, %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_negate_overflow(struct overflow_info* info, uintptr_t)
{
    printk_crit("UBSAN: subtraction overflow, %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_out_of_bounds(struct oob_info* info, uintptr_t)
{
    printk_crit("UBSAN: out of bounds array index %s (%d-bit), index type %s (%d-bit)\n", info->array_type->name, bitw(info->array_type->info), info->index_type->name, bitw(info->index_type->info));
    print_location(info->location); 
}

void __ubsan_handle_load_invalid_value(struct inv_val_info* info, uintptr_t)
{
    printk_crit("UBSAN: load-invalid-value: %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_divrem_overflow(struct overflow_info* info, uintptr_t, uintptr_t)
{
    printk_crit("UBSAN: divrem overflow, %s (%d-bit)\n", info->type->name, bitw(info->type->info));
    print_location(info->location);
}

void __ubsan_handle_shift_out_of_bounds(struct shft_oob_info* info, uintptr_t, uintptr_t)
{
    printk_crit("UBSAN: shift out of bounds, %s (%d-bit) shifted by %s (%d-bit)\n", info->lhs_type->name, bitw(info->lhs_type->info), info->rhs_type->name, bitw(info->rhs_type->info));
    print_location(info->location);
}

#define STACK_CHK_GUARD 0x595e9fbd94fda766

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail()
{
    panic("Stack smashing detected");
};

#endif

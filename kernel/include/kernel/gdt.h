#ifndef __ARCH_I386_GDT_H__

    #define __ARCH_I386_GDT_H__

    #include <stdint.h>

    //Present bit. needs to be set for valid segments.
    #define GDT_ACCESS_PR_BIT(x)     ((x) << 7)
    //Privilege bits. Ring 0-3.
    #define GDT_ACCESS_PRIVL_BITS(x) (((x) & 3) << 5)
    //Descriptor type. set for code/data descriptor.
    #define GDT_ACCESS_S_BIT(x)      ((x) << 4)
    //Executable bit. set for code selector.
    #define GDT_ACCESS_EX_BIT(x)     ((x) << 3)
    //Direction/Conforming bit.
    //Data selector: direction of growth, 0 = up, 1 = down (if 1 offset > limit needs to be true)
    //Code selector: conforming bit. if 0, code in this segment can only be ran by code in the same ring.
    #define GDT_ACCESS_DC_BIT(x)     ((x) << 2)
    //Readable/writeable bit.
    //Code selector: if set, can read. write never allowed
    //Data selector: if set, can write. read always allowed
    #define GDT_ACCESS_RW_BIT(x)     ((x) << 1)
    //Remaining bit is accessed bit. set to 0. CPU will set to 0 once segment is accessed.

    //Granularity bit. 0 = 1 byte, 1 = 1K
    #define GDT_FLAGS_GR(x) ((x) << 7)
    //Size bit. 0 = 16 bit selector, 1 = 32 bit selector
    #define GDT_FLAGS_SZ(x) ((x) << 6)
    //Reserved bit. should always be 0
    #define GDT_FLAGS_0(x)  ((x) << 5)
    //Available bit. should always be 0
    #define GDT_FLAGS_A(x)  ((x) << 4)

    struct gdt_entry {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_middle;
        uint8_t access;
        uint8_t flags;
        uint8_t base_high;
    } __attribute__((packed));

    struct gdtr_entry {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    void load_initial_gdt();

#endif

#ifndef __ARCH_I386_IDT_H__

    #define __ARCH_I386_IDT_H__

    #include <stdint.h>

    #define IDT_SIZE 256

    struct idt_entry {
        uint16_t addr_low;
        uint16_t kernel_cs;
        uint8_t reserved;
        uint8_t attributes;
        uint16_t addr_high;
    } __attribute__((packed));

    struct idtr_entry {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed));

    void load_initial_idt();

#endif

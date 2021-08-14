#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <kernel/idt.h>

#include "interrupts.h"

struct idtr_entry initial_idtr;
struct idt_entry initial_idt[IDT_SIZE];

extern void isr_wrapper_0();
extern void isr_wrapper_1();
extern void isr_wrapper_2();
extern void isr_wrapper_3();
extern void isr_wrapper_4();
extern void isr_wrapper_5();
extern void isr_wrapper_6();
extern void isr_wrapper_7();
extern void isr_wrapper_8();
extern void isr_wrapper_9();
extern void isr_wrapper_10();
extern void isr_wrapper_11();
extern void isr_wrapper_12();
extern void isr_wrapper_13();
extern void isr_wrapper_14();
extern void isr_wrapper_16();
extern void isr_wrapper_17();
extern void isr_wrapper_18();
extern void isr_wrapper_19();
extern void isr_wrapper_20();
extern void isr_wrapper_21();
extern void isr_wrapper_reserved();

void config_idt_entry(struct idt_entry *entry, uint32_t addr, uint16_t cs, uint8_t attributes){
    entry->addr_low = (uint16_t)addr;
    entry->addr_high = (uint16_t)(addr >> 16);

    entry->kernel_cs = cs;
    entry->reserved = 0;
    entry->attributes = attributes;
}

void load_initial_idt(){
    memset(initial_idt, 0, sizeof(struct idt_entry) * IDT_SIZE);

    initial_idtr.limit = sizeof(struct idt_entry) * IDT_SIZE - 1;
    initial_idtr.base = (uint32_t)&initial_idt[0];

    config_idt_entry(&initial_idt[0], (uint32_t)&isr_wrapper_0, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[1], (uint32_t)&isr_wrapper_1, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[2], (uint32_t)&isr_wrapper_2, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[3], (uint32_t)&isr_wrapper_3, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[4], (uint32_t)&isr_wrapper_4, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[5], (uint32_t)&isr_wrapper_5, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[6], (uint32_t)&isr_wrapper_6, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[7], (uint32_t)&isr_wrapper_7, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[8], (uint32_t)&isr_wrapper_8, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[9], (uint32_t)&isr_wrapper_9, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[10], (uint32_t)&isr_wrapper_10, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[11], (uint32_t)&isr_wrapper_11, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[12], (uint32_t)&isr_wrapper_12, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[13], (uint32_t)&isr_wrapper_13, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[14], (uint32_t)&isr_wrapper_14, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[15], (uint32_t)&isr_wrapper_reserved, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[16], (uint32_t)&isr_wrapper_16, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[17], (uint32_t)&isr_wrapper_17, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[18], (uint32_t)&isr_wrapper_18, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[19], (uint32_t)&isr_wrapper_19, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[20], (uint32_t)&isr_wrapper_20, 0x08, 0b10001110);
    config_idt_entry(&initial_idt[21], (uint32_t)&isr_wrapper_21, 0x08, 0b10001110);

    for(size_t i = 22; i < 32; i++){
        config_idt_entry(&initial_idt[i], (uint32_t)&isr_wrapper_reserved, 0x08, 0b10001110);
    }

    asm volatile("lidt %0;" : : "m" (initial_idtr) : "eax");
}

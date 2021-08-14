#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <kernel/idt.h>

#include "interrupts.h"

struct idtr_entry initial_idtr;
struct idt_entry initial_idt[IDT_SIZE];

extern void isr_wrapper_0();
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

    for(size_t i = 22; i < 32; i++){
        config_idt_entry(&initial_idt[i], (uint32_t)&isr_wrapper_reserved, 0x08, 0b10001110);
    }

    asm volatile("lidt %0;" : : "m" (initial_idtr) : "eax");
}

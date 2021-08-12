#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <kernel/mem.h>
#include <kernel/gdt.h>

struct gdtr_entry initial_gdtr;
struct gdt_entry initial_gdt[5];

void config_gdt_entry(struct gdt_entry *entry, uint32_t base, uint32_t limit,
                        uint8_t access, uint8_t flags){
    entry->base_low = (uint16_t)base;
    entry->base_middle = (uint8_t)(base >> 16);
    entry->base_high = (uint8_t)(base >> 24);
    
    entry->limit_low = (uint16_t)limit;
    entry->flags = flags & 0b11110000;
    
    entry->flags |= (limit >> 16) & 0b00001111;
    entry->access = access;
}

void load_initial_gdt(){
    uint8_t flags = GDT_FLAGS_GR(1) | GDT_FLAGS_SZ(1);

    config_gdt_entry(&initial_gdt[0], 0, 0, 0, 0);
    config_gdt_entry(&initial_gdt[1], 0, (uint32_t)~0, 0b10011010, flags);
    config_gdt_entry(&initial_gdt[2], 0, (uint32_t)~0, 0b10010010, flags);
    config_gdt_entry(&initial_gdt[3], 0, (uint32_t)~0, 0b11111010, flags);
    config_gdt_entry(&initial_gdt[4], 0, (uint32_t)~0, 0b11110010, flags);

    initial_gdtr.limit = sizeof(struct gdt_entry) * 5 - 1;
    initial_gdtr.base = (uint32_t)&initial_gdt[0];

    asm volatile("lgdt %0;"
                 "mov  $0x10, %%ax;"
                 "mov  %%ax, %%ds;"
                 "mov  %%ax, %%es;"
                 "mov  %%ax, %%fs;"
                 "mov  %%ax, %%gs;"
                 "ljmp $0x08, $1f;"
                 "1:" : : "m" (initial_gdtr) : "eax");
}

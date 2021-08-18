#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/mem.h>
#include <kernel/early_pmm_init.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>

#if defined(__linux__)
#error "A cross compiler is required to compile the kernel."
#endif
 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

extern void iprint(uint64_t);

void kernel_main(){
    late_pmm_init(get_early_pmm());
    load_initial_gdt();
    //Map VGA buffer in virtual memory. If mapping fails, silently exit.
    if(map_page((void*)0x000B8000, (void*)0xC03FE000, 0x103)){
        return;
    }

    terminal_initialize();

    terminal_writestring("something or another\n");

    if((uint32_t)(get_early_pmm().bitmap) % PAGE_SIZE == 0){
        terminal_writestring("amazing\n");
    }

    if(get_page() != get_page()){
        terminal_writestring("get_page() probably works partially at least\n");
    }

    load_initial_idt();

    pic_init();

    while(1){};
}

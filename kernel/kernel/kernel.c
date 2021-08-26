#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/klimits.h>
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

    asm volatile ("sti;");

    init_heap();

    char *test = kmalloc(50);

    memset(test, 0, 50);
    strcpy(test, "Hello heap world!\n");
    terminal_writestring(test);

    char *test2 = kmalloc(12);
    memset(test2, 0, 12);
    strcpy(test2, "Hello 2\n");
    terminal_writestring(test2);

    char *test3 = kmalloc(20);
    memset(test3, 0, 20);
    strcpy(test3, "Hello 3\n");
    terminal_writestring(test3);

    char *big = kmalloc(4096);
    memset(big, 0, 4096);
    char *smth = big + 1000;
    strcpy(smth, "Hello big heap!\n");
    terminal_writestring(smth);

    kfree(test);
    kfree(test2);
    kfree(test3);
    kfree(big);

    char *test4 = krealloc(test, 100);
    terminal_writestring(test4);
    memset(test4, 0, 100);
    strcpy(test4, "And now for something longer...\n");
    terminal_writestring(test4);

    while(1){};
}

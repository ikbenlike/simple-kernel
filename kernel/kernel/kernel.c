#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/mem.h>

#if defined(__linux__)
#error "A cross compiler is required to compile the kernel."
#endif
 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void kernel_main(void){
    terminal_initialize();

    void *pa = get_physaddr(0xC03FE000);

    if(pa == (void*)(0x000B8000)){
        terminal_writestring("aye");
    }
    else if(pa == (void*)0){
        terminal_writestring("nah");
    }
}

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/tty.h>

#if defined(__linux__)
#error "A cross compiler is required to compile the kernel."
#endif
 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void *get_physaddr(void *virtualaddr){
    uint32_t *pd = (uint32_t*)0xFFFFF000;

    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0b1111111111;
    uint32_t offset = (uint32_t)virtualaddr & 0b111111111111;

    return *(uint32_t*)(0xFFC00000 | pdindex << 12 | ptindex << 2) & ~0b111111111111 | offset;
}

void kernel_main(void){
    terminal_initialize();

    void *pa = get_physaddr(0xC03FE000);

    if(pa == (void*)(0x000B8000)){
        terminal_writestring("aye");
    }
}

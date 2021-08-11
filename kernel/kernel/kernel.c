#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/mem.h>

#if defined(__linux__)
#error "A cross compiler is required to compile the kernel."
#endif
 
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void kernel_main(/*uint32_t multiboot_magic, void *multiboot_header*/){
    //Map VGA buffer in virtual memory. If mapping fails, silently exit.
    if(map_page((void*)0x000B8000, (void*)0xC03FE000, 0x103)){
        return;
    }

    terminal_initialize();

    terminal_writestring("something or another\n");

    /*if(multiboot_magic != MULTIBOOT_MAGIC){
        terminal_writestring("This kernel requires a multiboot-compliant bootloader.");
    }

    // Physical address. Must first be mapped to virtual page to be useful for us.
    struct multiboot_info *mbinfo = (struct multiboot_info*)multiboot_header;
    //init_frame_allocator(mbinfo);*/


    /*void *pa = get_physaddr((void*)0xC03FE000);

    if(pa == (void*)(0x000B8000)){
        terminal_writestring("aye");
    }
    else if(pa == (void*)0){
        terminal_writestring("nah");
    }*/
}

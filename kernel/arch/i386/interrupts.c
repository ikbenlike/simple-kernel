#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/idt.h>

extern void iprint(uint64_t);

void zero_division_handler(struct stackframe sf){
    if((sf.cs & 0b11) == 3){
        //TODO when userspace gets added: terminate offending program
    }
    else {
        terminal_writestring("FATAL ERROR: division-by-zero occurred in kernel-space.");
        while(1){};
    }
}

void reserved_interrupt(struct stackframe fs){
    if((sf.cs & 0b11) == 3){
        //TODO terminate program
    }
    terminal_writestring("FATAL ERROR: this isn't supposed to happen! Reserved interrupt fired.");

    while(1){};
}

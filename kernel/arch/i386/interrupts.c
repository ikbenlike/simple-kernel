#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/idt.h>


void interrupt_handler(struct stackframe sf){
    terminal_writestring("interrupt fired!\n");
    if(sf.err == 0){
        terminal_writestring("error seems to be working too\n");
    }
}

void reserved_interrupt(struct stackframe fs){
    terminal_writestring("FATAL ERROR: this isn't supposed to happen! Reserved interrupt fired.");

    while(1){};
}

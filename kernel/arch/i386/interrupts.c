#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/idt.h>


void interrupt_handler(struct stackframe sf){
    terminal_writestring("interrupt fired!\n");
    if(sf.err == 0){
        terminal_writestring("error seems to be working too\n");
    }
}

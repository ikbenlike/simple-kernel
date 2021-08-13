#include <stdint.h>

#include <kernel/tty.h>


void interrupt_handler(){
    terminal_writestring("interrupt fired!\n");
}

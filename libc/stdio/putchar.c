#include <stdio.h>
#include <kernel/tty.h>

int putchar(int ca){
    char c = (char)ca;
    terminal_write(&c, sizeof(c));
    return ca;
}

#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void){
    puts("abort()");
    while(1){}
}

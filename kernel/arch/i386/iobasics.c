#include <stdint.h>

void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %0, %1" : : "a"(value), "d" (port) : "memory");
}

uint8_t inb(uint16_t port){
    uint8_t r = 0;
    asm volatile ("inb %1, %0" : "=a" (r) : "d" (port) : "memory");
    return r;
}

void outl(uint16_t port, uint32_t value){
    asm volatile ("outl %0, %1" : : "a"(value), "d" (port) : "memory");
}

uint32_t inl(uint16_t port){
    uint32_t r = 0;
    asm volatile ("inl %1, %0" : "=a" (r) : "d" (port) : "memory");
    return r;
}

void iowait(){
    inb(0x80);
}

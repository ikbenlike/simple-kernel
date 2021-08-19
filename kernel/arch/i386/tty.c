#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "vga.h"

#if defined(__linux__) || !defined(__i386__)
#error "An i386 cross compiler is necessary to compile this module."
#endif

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t *const VGA_MEMORY = (uint16_t*)0xC03FE000;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_initialize(void){
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    for(size_t y = 0; y < VGA_HEIGHT; y++){
        for(size_t x = 0; x < VGA_WIDTH; x++){
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color){
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y){
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_delete_last_line(){
    uint16_t *line = terminal_buffer + VGA_WIDTH * (VGA_HEIGHT - 1);
    for(size_t i = 0; i < VGA_WIDTH; i++){
        line[i] = vga_entry(' ', terminal_color);
    }
}

void terminal_scroll(){
    for(size_t i = 0; i < VGA_HEIGHT - 1; i++){
        void *dest = (void*)(terminal_buffer + VGA_WIDTH * i);
        void *src = (void*)(terminal_buffer + VGA_WIDTH * (i + 1));
        memcpy(dest, src, VGA_WIDTH * sizeof(uint16_t));
    }
    terminal_delete_last_line();
}

void terminal_putchar(char c){
    if(c == '\n'){
        terminal_row++;
        terminal_column = 0;
    }
    else {
        terminal_putentryat(c, terminal_color, terminal_column++, terminal_row);
        if(terminal_column == VGA_WIDTH){
            terminal_row++;
            terminal_column = 0;
        }
    }

    if(terminal_row == VGA_HEIGHT){
        terminal_scroll();
        terminal_row--;
    }
}

void terminal_write(const char *data, size_t size){
    for(size_t i = 0; i < size; i++) terminal_putchar(data[i]);
}

void terminal_writestring(const char *data){
    terminal_write(data, strlen(data));
}

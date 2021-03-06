#include <stdint.h>

#include <kernel/pic.h>

extern void iprint(uint64_t);

void pic_send_eoi(uint8_t irq){
    if(irq > 7){
        outb(PIC2_COMMAND, PIC_COMMAND_EOI);
    }

    outb(PIC1_COMMAND, PIC_COMMAND_EOI);
}

void pic_set_irq_mask(uint8_t line){
    uint16_t port = PIC1_DATA;

    if(line > 7){
        port = PIC2_DATA;
        line -= 8;
    }

    outb(port, inb(port) | (1 << line));  
}

void pic_clear_irq_mask(uint8_t line){
    uint16_t port = PIC1_DATA;

    if(line > 7){
        port = PIC2_DATA;
        line -= 8;
    }

    outb(port, inb(port) & ~(1 << line));
}

void pic_map(uint8_t offset1, uint8_t offset2, uint8_t m1, uint8_t m2){
    //uint8_t mask1 = inb(PIC1_DATA);
    //uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    iowait();
    outb(PIC1_DATA, offset1);
    iowait();
    outb(PIC1_DATA, 0b00000100);
    iowait();
    outb(PIC1_DATA, PIC_ICW4_8086);
    iowait();
    
    outb(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    iowait();
    outb(PIC2_DATA, offset2);
    iowait();
    outb(PIC2_DATA, 0b00000010);
    iowait();
    outb(PIC2_DATA, PIC_ICW4_8086);
    iowait();

    outb(PIC1_DATA, m1);
    outb(PIC2_DATA, m2);
}

void pic_disable(){
    uint16_t mask = 0xFF;

    outb(PIC1_DATA, mask);
    outb(PIC2_DATA, mask);
}

void pic_init(){
    pic_map(PIC_OFFSET1, PIC_OFFSET2, 0b11111001, ~0U);
}

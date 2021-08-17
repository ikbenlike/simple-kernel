#ifndef __ARCH_I386_BASICIO_H__

    #define __ARCH_I386_BASICIO_H__

    #include <stdint.h>

    #define PIC1_BASE           0x20
    #define PIC2_BASE           0xA0
    #define PIC1_COMMAND        PIC1_BASE
    #define PIC1_DATA           (PIC1_BASE+1)
    #define PIC2_COMMAND        PIC2_BASE
    #define PIC2_DATA           (PIC2_BASE+1)

    #define PIC_COMMAND_EOI     0x20

    #define PIC_OFFSET1         0x20
    #define PIC_OFFSET2         0x28

    #define PIC_ICW1_ICW4       0x01
    #define PIC_ICW1_SINGLE4    0x02
    #define PIC_ICW1_INTERVAL4  0x04
    #define PIC_ICW1_LEVEL      0x08
    #define PIC_ICW1_INIT       0x10

    #define PIC_ICW4_8086       0x01
    #define PIC_ICW4_AUTO       0x02
    #define PIC_ICW4_BUF_SLAVE  0x08
    #define PIC_ICW4_BUF_MASTER 0x0C
    #define PIC_ICW4_SFNM       0x10

    void outb(uint16_t port, uint8_t value);
    uint8_t inb(uint16_t port);
    void iowait();

    void pic_send_eoi(uint8_t irq);
    void pic_set_irq_mask(uint8_t line);
    void pic_clear_irq_mask(uint8_t line);
    void pic_map(uint8_t offset1, uint8_t offset2);
    void pic_disable();

#endif

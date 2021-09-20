#ifndef __ARCHI386_IOBASICS_H__

    #define __ARCHI386_IOBASICS_H__

    #include <stdint.h>

    void outb(uint16_t port, uint8_t value);
    uint8_t inb(uint16_t port);

    void outl(uint16_t port, uint32_t value);
    uint32_t inl(uint16_t port);

    void iowait();

#endif

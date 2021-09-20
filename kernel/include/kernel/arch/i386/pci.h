#ifndef __ARCH_I386_PCI_H__

    #define __ARCH_I386_PCI_H__

    #define PCI_CONFIG_ADDRESS 0xCF8
    #define PCI_CONFIG_DATA    0xCFC

    #include <stdint.h>

    uint16_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg);

#endif

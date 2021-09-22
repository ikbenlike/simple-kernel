#include <stdint.h>
#include <stdbool.h>

#include <kernel/iobasics.h>
#include <kernel/pci.h>

/*

    +--------+----------+-------+--------+----------+----------+
    | Enable | Reserved |  Bus  | Device | Function | Register |
    +--------+----------+-------+--------+----------+----------+
    | bit 31 |  30-24   | 23-16 | 15-11  |   10-8   |   7-0    |
    +--------+----------+-------+--------+----------+----------+

*/

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t reg){
    uint32_t address = 0x80000000; //Set all bits except enable bit to 0.

    address |= (bus << 16) | (slot << 11) | (func << 8) | (reg & ~0x3);
    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_DATA);
}

bool pci_read_device_base(uint8_t bus, uint8_t slot, uint8_t func, struct pci_device_base *data){
    
}

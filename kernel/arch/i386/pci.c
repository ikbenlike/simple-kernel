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
    uint32_t device_and_vendor = pci_read_config(bus, slot, func, PCI_DEVICE_AND_VENDOR);

    //If response is all ones, device doesn't exist. Return false for failure.
    if(device_and_vendor = 0xFFFFFFFF) return false;

    uint32_t status_and_command = pci_read_config(bus, slot, func, PCI_STATUS_AND_COMMAND);
    uint32_t class_and_subclass = pci_read_config(bus, slot, func, PCI_CLASS_SUB_PROG_REVISION);
    uint32_t bist_and_head_etc = pci_read_config(bus, slot, func, PCI_BIST_HEAD_LAT_CACHE);

    data->vendor_id = (uint16_t)device_and_vendor;
    data->device_id = (uint16_t)(device_and_vendor >> 16);

    data->command = (uint16_t)status_and_command;
    data->status = (uint16_t)(status_and_command >> 16);

    data->revision_id = (uint8_t)class_and_subclass;
    data->prog_if = (uint8_t)(class_and_subclass >> 8);
    data->subclass = (uint8_t)(class_and_subclass >> 16);
    data->class = (uint8_t)(class_and_subclass >> 24);

    data->cache_line_size = (uint8_t)bist_and_head_etc;
    data->latency_timer = (uint8_t)(bist_and_head_etc >> 8);
    data->header_type = (uint8_t)(bist_and_head_etc >> 16);
    data->bist = (uint8_t)(bist_and_head_etc >> 24);

    return true;
}

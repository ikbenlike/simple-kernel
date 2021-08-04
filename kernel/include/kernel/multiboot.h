#ifndef __ARCH_I386_MULTIBOOT_H__

    #define __KERNEL_MULTIBOOT_H__

    #include <stdint.h>

    #define MULTIBOOT_MAGIC 0x2BADB002

    //Any value not specified in this enum indicates a reserved area
    enum MmapEntryType {
        MEM_AVAILABLE = 1,
        MEM_ACPI = 3,
        MEM_PRESERVE = 4,
        MEM_UNAVAILABLE = 5,
    };

    struct multiboot_aout_table {
        uint32_t tabsize;
        uint32_t strsize;
        uint32_t addr;
        uint32_t reserved;
    };

    struct multiboot_elf_table {
        uint32_t num;
        uint32_t size;
        uint32_t addr;
        uint32_t shndx;
    };

    struct multiboot_info {
        uint32_t flags;

        uint32_t mem_lower;
        uint32_t mem_upper;

        uint32_t boot_device;

        uint32_t cmdline;

        uint32_t mods_count;
        uint32_t mods_addr;

        union {
            struct multiboot_aout_table aout;
            struct multiboot_elf_table elf;
        } syms;

        uint32_t mmap_length;
        uint32_t mmap_addr;

        uint32_t drives_length;
        uint32_t drives_addr;

        uint32_t config_table;

        uint32_t boot_loader_name;

        uint32_t apm_table;

        uint32_t vbe_control_info;
        uint32_t vbe_mode_info;
        uint32_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
    };

    struct multiboot_mmap_entry {
        uint32_t size;
        uint64_t addr;
        uint64_t length;
        uint32_t type;
    } __attribute__((packed));

#endif

ENTRY(_start)

SECTIONS
{
    . = 1M;

    _kernel_start = .;

    .multiboot.data : {
        *(.multiboot.data)
    }

    .multiboot.text : {
        *(.multiboot.text)
    }

    .multiboot.stack : {
        *(.multiboot.stack)
    }

    . += 0xC0000000;

    .text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
    {
        *(.rodata)
    }

    _kernel_nowrite_end = .;

    .data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.initial_stack)
    }

    _kernel_end = .;
}

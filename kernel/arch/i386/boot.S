.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, ALIGN | MEMINFO
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .initial_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16385
stack_top:

.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096



.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
    movl $(boot_page_table1 - 0xC0000000), %edi
    movl $0, %esi
    movl $1023, %ecx

1:
    cmpl $_kernel_start, %esi
    jl 3f
    cmpl $(_kernel_end - 0xC0000000), %esi
    jge 4f
    cmpl $(_kernel_nowrite_end - 0xC0000000), %esi
    jge 2f

    movl %esi, %edx
    orl $0x001, %edx
    movl %edx, (%edi)
    jmp 3f

2:
    movl %esi, %edx
    orl $0x003, %edx
    movl %edx, (%edi)

3:
    addl $4096, %esi
    addl $4, %edi
    loop 1b

4:
    xor %edi, %edi
    addl $4, %edi

5:
    cmpl $(boot_page_directory - 0xC0000000 + 4 * 1023), %edi
    jle 6f
    movl $0, (%edi)
    addl $4, %edi
    loop 5b

6:
    movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1022 * 4

    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

    movl $(boot_page_directory - 0xC0000000), %edx
    orl $0x003, %edx
    movl %edx, boot_page_directory - 0xC0000000 + 1023 * 4

    movl $(boot_page_directory - 0xC0000000), %ecx
    movl %ecx, %cr3

    movl %cr0, %ecx
    orl $0x80010000, %ecx
    movl %ecx, %cr0

    lea 7f, %ecx
    jmp *%ecx


.section .text

7:
    movl $0, boot_page_directory + 0

    movl %cr3, %ecx
    movl %ecx, %cr3

    mov $stack_top, %esp

    call kernel_main

    cli
8:  hlt
    jmp 5b 

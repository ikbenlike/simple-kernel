.section .text
.global _flush_full_tlb
.type flush_full_tlb, @function
_flush_full_tlb:
    movl %cr3, %ecx
    movl %ecx, %cr3
    ret

.global gdt_load
.type gdt_load, @function
gdt_load:
    pop %eax
    lgdt (%eax)
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $8, $1f
1:
    ret

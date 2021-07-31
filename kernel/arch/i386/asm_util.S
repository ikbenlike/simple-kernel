.section .text
.global _flush_full_tlb
.type flush_full_tlb, @function
_flush_full_tlb:
    movl %cr3, %ecx
    movl %ecx, %cr3
    ret

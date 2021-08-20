#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/mem.h>

#include "util.h"

struct managed_memory pmm = (struct managed_memory){.bitmap = (void*)0, .size = 0};

extern char _kernel_start;
extern char _kernel_end;

const char *kernel_start_p = &_kernel_start;
const char *kernel_end_p = &_kernel_end - 0xC0000000;

extern void _flush_full_tlb();

void flush_full_tlb(){
    _flush_full_tlb();
}

void *get_physaddr(void *virtualaddr){
    uint32_t *pd = (uint32_t*)0xFFFFF000;

    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0b1111111111;
    uint32_t offset = (uint32_t)virtualaddr & 0b111111111111;

    uint32_t pt = *(uint32_t*)(0xFFFFF000 | pdindex << 2);
    if(!(pt & 1)){
        return (void*)0;
    }

    uint32_t page = *(uint32_t*)(0xFFC00000 | pdindex << 12 | ptindex << 2);
    if(!(page & 1)){
        return (void*)0;
    }

    return (void*)((page & ~0b111111111111) | offset);
}

int map_page(void *physaddr, void *virtualaddr, uint16_t flags){
    uint32_t pa = (uint32_t)physaddr;
    uint32_t va = (uint32_t)virtualaddr;

    if(pa % PAGE_SIZE){
        return MAP_PAGE_PHYSADDR_UNALIGNED;
    }
    else if(va % PAGE_SIZE){
        return MAP_PAGE_VIRTUALADDR_UNALIGNED;
    }

    uint32_t pdindex = va >> 22;
    uint32_t ptindex = va >> 12 & 0b1111111111;

    //uint32_t *pd = (uint32_t*)0xFFFFF000;
    // TODO: dynamically add PDE when no PT yet present in this location

    uint32_t *pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
    if(pt[ptindex] & 1){
        return MAP_PAGE_MAPPING_ALREADY_PRESENT;
    }

    pt[ptindex] = pa | (flags & 0b111111111111) | 1;

    flush_full_tlb();

    return 0;
}

int unmap_page(void *virtualaddr){
    uint32_t va = (uint32_t)virtualaddr;

    uint32_t pdindex = va >> 22;
    uint32_t ptindex = va >> 12 & 0b1111111111;

    uint32_t *pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);
    pt[ptindex] = (uint32_t)~0;

    flush_full_tlb();

    return 0;
}

void iprint(uint64_t n){
    if( n > 9 ){
        uint64_t a = n / 10;

        n -= 10 * a;
        iprint(a);
    }
    terminal_putchar('0'+n);
}

uint8_t first_zero_in_byte(uint8_t b){
    if(b == (uint8_t)~0){
        return 8;
    }

    for(uint8_t i = 0; i < 8; i++){
        if((b & (1 << i)) == 0) return i;
    }

    return 8;
}

inline void physaddr_to_offsets(void *physaddr, uint32_t *bm_index, uint32_t *offset){
    uint32_t x = (uint32_t)physaddr;
    uint32_t y = x / PAGE_SIZE;
    *bm_index = y / 8;
    *offset = y % 8;
}

inline void *offsets_to_physaddr(uint32_t bm_index, uint32_t offset){
    return (void*)(PAGE_SIZE * (bm_index * 8 + offset));
}

bool get_page_state(void *physaddr){
    uint32_t index = 0;
    uint32_t offset = 0;
    physaddr_to_offsets(physaddr, &index, &offset);

    return pmm.bitmap[index] & (1 << offset);
}

void *get_page(){
    pmm.bitmap[pmm.next_index] |= 1 << pmm.next_offset;
    void *r = offsets_to_physaddr(pmm.next_index, pmm.next_offset);

    for(uint32_t i = pmm.next_index; i < pmm.size; i++){
        if(pmm.bitmap[i] < (uint8_t)~0){
            uint8_t j = first_zero_in_byte(pmm.bitmap[i]);
            pmm.next_offset = i;
            pmm.next_index = j;
            break;
        }
    }

    return r;
}

void free_page(void *physaddr){
    uint32_t index = 0;
    uint32_t offset = 0;
    physaddr_to_offsets(physaddr, &index, &offset);

    pmm.bitmap[index] &= ~(1 << offset);
    pmm.next_index = index;
    pmm.next_offset = offset;
}


void late_pmm_init(struct managed_memory p){
    pmm = p;

    const uint32_t kernel_size = (uint32_t)(kernel_end_p - kernel_start_p);
    const uint32_t kernel_page_count = div_ceil(kernel_size, PAGE_SIZE);
    const uint32_t combined_page_count = kernel_page_count + div_ceil(pmm.size, PAGE_SIZE);

    uint32_t bitmap_virtaddr = 0xC0000000 | kernel_page_count << 12;
    for(size_t i = kernel_page_count; i < combined_page_count; i++){
        uint32_t va = 0xC0000000 | i << 12;
        uint32_t pa = (uint32_t)pmm.bitmap + (i - kernel_page_count) * PAGE_SIZE;
        map_page((void*)pa, (void*)va, 0x103);
    }

    pmm.bitmap = (uint8_t*)bitmap_virtaddr;
    pmm.set_up = true;
}

/*
    Kernel starts being mapped at PDE 768, or address 0xC0000000,
    meaning there is 1GB of space reserved for kernel. Reserve 
    half of this for the kernel heap, for now. However, take note
    of the PDE which is recursively mapped at 0xFFC00000.
*/

struct heap_element {
    struct heap_element *next;
    struct heap_element *prev;
};

const char *heap_start = (char*)0xE0000000;
const char *heap_end = (char*)(0xFFC00000 - sizeof(struct heap_element));

void init_heap(){
    struct heap_element *start = (struct heap_element*)heap_start;
    struct heap_element *end = (struct heap_element*)heap_end;

    start->next = end;
    start->prev = (void*)0;

    end->next = (void*)0;
    end->prev = start;
}

size_t get_area_size(struct heap_element *area){
    if(area->next == (void*)0)
        return 0;

    return (uint32_t)area->next - (uint32_t)area;
}

void *kmalloc(size_t size){

}

void kfree(void *ptr){

}

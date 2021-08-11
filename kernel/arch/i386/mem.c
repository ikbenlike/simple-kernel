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

}

void free_page(void *physaddr){
    uint32_t index = 0;
    uint32_t offset = 0;
    physaddr_to_offsets(physaddr, &index, &offset);

    pmm.bitmap[index] &= ~(1 << offset);
    pmm.last_index = index;
    pmm.last_offset = offset;
}

/*void sanitize_available(struct multiboot_mmap_entry *e, uintptr_t start, const uintptr_t end){
    if(e->type != MEM_AVAILABLE) return;
    uintptr_t cur_addr = start;
    struct multiboot_mmap_entry *lowest_reserved = (void*)0;
    while(cur_addr < end){
        struct multiboot_mmap_entry *cur_entry = (struct multiboot_mmap_entry*)cur_addr;
        if(cur_entry->type != MEM_AVAILABLE){
            if(e->addr < cur_entry->addr && e->addr + e->length > cur_entry->addr){
                if(lowest_reserved == (void*)0 || lowest_reserved->addr > cur_entry->addr){
                    lowest_reserved = cur_entry;
                    e->length = cur_entry->addr - e->addr;
                    terminal_writestring("sanitized area at address ");
                    iprint(e->addr);
                    terminal_putchar('\n');
                }
            }
        }
        cur_addr += cur_entry->size + sizeof(uintptr_t);
    }
    if(e->addr % PAGE_SIZE != 0){
        e->addr += PAGE_SIZE - (e->addr % PAGE_SIZE);
        e->length -= PAGE_SIZE - ((e->addr + e->length) % PAGE_SIZE);
        terminal_writestring("sanitized address and lenght of area at address ");
        iprint(e->addr);
        terminal_putchar('\n');
    }
}*/

/*void init_frame_allocator(struct multiboot_info *mbh_physaddr){
    uint32_t offset = (uint32_t)mbh_physaddr % PAGE_SIZE;
    void *mbh_page_start = (void*)((uint32_t)mbh_physaddr - offset);
    if(map_page(mbh_page_start, (void*)0xC03FF000, 0x103) == 0){
        terminal_writestring("multiboot header page mapped\n");
    }

    struct multiboot_info *mbh_vaddr = (struct multiboot_info*)(0xC03FF000 + offset);

    if((mbh_vaddr->flags & (1<<6)) == 0){
        terminal_writestring("multiboot failed to provide memory map\n");
        return;
    }

    terminal_writestring("multiboot provided a memory map\n");

    uint32_t mmap_page_start = mbh_vaddr->mmap_addr - (mbh_vaddr->mmap_addr % PAGE_SIZE);
    uint32_t end_page = mmap_page_start + mbh_vaddr->mmap_length + (PAGE_SIZE - (mbh_vaddr->mmap_length % PAGE_SIZE));

    uint32_t pagen = 900; // page at which to start mapping multiboot mmap
    uint32_t i = 0;
    for(uint32_t cur_page = mmap_page_start; cur_page + i * PAGE_SIZE < end_page; i++){
        void *pa = (void*)(mmap_page_start + i * PAGE_SIZE);
        void *va = (void*)(0xC0000000 | ((pagen + i) << 12));
        int x = map_page(pa, va, 0x1);
        //int x = map_page((void*)(mmap_page_start + i * PAGE_SIZE), (void*)(0xC0000000 | ((pagen + i) << 12)), 0x1);
        if(x == MAP_PAGE_PHYSADDR_UNALIGNED){
            terminal_writestring("physical address unaligned\n");
        }
        else if(x == MAP_PAGE_VIRTUALADDR_UNALIGNED){
            terminal_writestring("virtual address unaligned\n");
        }
        else if(x == MAP_PAGE_MAPPING_ALREADY_PRESENT){
            terminal_writestring("mapping already present\n");
        }
    }

    const struct multiboot_mmap_entry *mmap_start = 0xC0384000 | (mbh_vaddr->mmap_addr % PAGE_SIZE);

    terminal_writestring("amount of pages mapped: ");
    iprint(i);
    terminal_putchar('\n');

    const uint32_t kernel_size = (uint32_t)(kernel_end_p - kernel_start_p);
    const uint32_t kernel_page_count = div_ceil(kernel_size, PAGE_SIZE);
    //Also functions as offset into current page table.

    uintptr_t cur_addr = (uintptr_t)mmap_start;
    const uintptr_t mmap_end = cur_addr + mbh_vaddr->mmap_length;
    struct multiboot_mmap_entry *kernel_entry = (void*)0;

    uint64_t last_addr = 0;
    uint64_t last_size = 0;
    while(cur_addr < mmap_end){
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry*)cur_addr;
        if((uint32_t)kernel_start_p >= e->addr && e->length >= kernel_size){
            kernel_entry = e;
        }
        terminal_writestring("memory area of size ");
        iprint(e->length);
        terminal_writestring(" starting at address ");
        iprint(e->addr);
        terminal_writestring(" of type ");
        iprint(e->type);
        terminal_putchar('\n');
        sanitize_available(e, (uintptr_t)mmap_start, mmap_end);

        if(e->type == MEM_AVAILABLE){
            last_addr = e->addr;
            last_size = e->length;
        }
        cur_addr += e->size + sizeof(uintptr_t);
    }

    uint64_t total_size = last_addr + last_size;
    /*terminal_writestring("total size: ");
    iprint(total_size);
    terminal_putchar('\n');
    terminal_writestring("distance between begin and end: ");
    iprint((last_addr+last_size) - first_addr);
    terminal_putchar('\n');*/

    /*const uint64_t page_count = total_size / PAGE_SIZE;
    const uint32_t bitmap_size = div_ceil(page_count, 8);
    const uint32_t bitmap_page_count = div_ceil(bitmap_size, PAGE_SIZE);
    const uint32_t combined_page_count = kernel_page_count + bitmap_page_count;
    const char *combined_end_p = kernel_start_p + PAGE_SIZE * combined_page_count;

    uint32_t bitmap_physaddr = 0;
    if(kernel_entry != (void*)0 && kernel_entry->length / PAGE_SIZE >= combined_page_count){
        bitmap_physaddr = (uint32_t)kernel_start_p + combined_page_count * PAGE_SIZE;
    }
    else {
        terminal_writestring("kernel area not suitable!\n");
        return; // TODO: handle finding another section in memory
    }

    uint32_t bitmap_virtaddr = 0xC0000000 | kernel_page_count << 12;
    for(size_t i = kernel_page_count; i < combined_page_count; i++){
        uint32_t va = 0xC0000000 | i << 12;
        uint32_t pa = bitmap_physaddr + (i - kernel_page_count) * PAGE_SIZE;
        if(map_page((void*)pa, (void*)va, 0x103)){
            terminal_writestring("mapping error at bitmap\n");
        }
    }

    pmm.bitmap = (uint8_t*)bitmap_virtaddr;
    pmm.size = bitmap_size;
    pmm.last_index = 0;
    pmm.last_offset = 0;

    memset(pmm.bitmap, (uint8_t)~0, pmm.size);

    bool pmm_last_set = false;
    cur_addr = (uintptr_t)mmap_start;
    while(cur_addr < mmap_end){
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry*)cur_addr;
        if(e->type == MEM_AVAILABLE){
            uint32_t npages = e->length / PAGE_SIZE;
            for(uint32_t i = 0; i < npages; i++){
                char *page_addr = (char*)(e->addr + i * PAGE_SIZE);
                if(kernel_start_p <= page_addr && page_addr < combined_end_p)
                    continue;

                free_page(page_addr);

                if(pmm_last_set == false){
                    uint32_t index = 0;
                    uint32_t offset = 0;
                    physaddr_to_offsets(page_addr, &index, &offset);

                    pmm.last_index = index;
                    pmm.last_offset = offset;
                    pmm_last_set = true;
                }
            }
        }
        cur_addr += e->size + sizeof(uintptr_t);
    }

    terminal_writestring("total memory size: ");
    iprint(total_size);
    terminal_putchar('\n');
    terminal_writestring("amount of pages: ");
    iprint(total_size / PAGE_SIZE);
    terminal_putchar('\n');
    terminal_writestring("required bitmap size: ");
    iprint(div_ceil(total_size / PAGE_SIZE, 8));
    terminal_putchar('\n');
    terminal_writestring("pages required for bitmap: ");
    iprint(div_ceil(bitmap_size, PAGE_SIZE));
    terminal_putchar('\n');
}*/

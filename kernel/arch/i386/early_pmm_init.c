#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <kernel/multiboot.h>
#include <kernel/mem.h>

#include "util.h"

struct managed_memory early_pmm;

struct managed_memory get_early_pmm(){
    return early_pmm;
}

__attribute__((section(".multiboot.text")))
inline void early_physaddr_to_offsets(void *physaddr, uint32_t *bm_index, uint32_t *offset){
    uint32_t x = (uint32_t)physaddr;
    uint32_t y = x / PAGE_SIZE;
    *bm_index = y / 8;
    *offset = y % 8;
}

__attribute__((section(".multiboot.text")))
void sanitize_available(struct multiboot_mmap_entry *e, uintptr_t start, const uintptr_t end){
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
                }
            }
        }
        cur_addr += cur_entry->size + sizeof(uintptr_t);
    }
    if(e->addr % PAGE_SIZE != 0){
        e->addr += PAGE_SIZE - (e->addr % PAGE_SIZE);
        e->length -= PAGE_SIZE - ((e->addr + e->length) % PAGE_SIZE);
    }
}

__attribute__((section(".multiboot.text")))
void init_frame_allocator(uint32_t multiboot_magic, struct multiboot_info *mbh){
    if(multiboot_magic != MULTIBOOT_MAGIC){
        while(1){};
    }

    if((mbh->flags & (1<<6)) == 0){
        return;
    }

    //Only here to satisfy gcc.
    typedef char symbol[];

    extern symbol _kernel_start;
    extern symbol _kernel_end;

    const uint32_t MB = 1048576;

    const char *kernel_start_p = _kernel_start;
    const char *kernel_end_p = _kernel_end - 0xC0000000;

    const uint32_t kernel_size = (uint32_t)(kernel_end_p - kernel_start_p);
    const uint32_t kernel_page_count = div_ceil(kernel_size, PAGE_SIZE);

    const struct multiboot_mmap_entry *mmap_start = (struct multiboot_mmap_entry*)mbh->mmap_addr;
    uintptr_t cur_addr = (uintptr_t)mmap_start;
    const uintptr_t mmap_end = cur_addr + mbh->mmap_length;
    struct multiboot_mmap_entry *kernel_entry = (void*)0;

    uint64_t last_addr = 0;
    uint64_t last_size = 0;
    while(cur_addr < mmap_end){
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry*)cur_addr;
        if((uint32_t)kernel_start_p >= e->addr && e->length >= kernel_size){
            kernel_entry = e;
        }
        sanitize_available(e, (uintptr_t)mmap_start, mmap_end);

        if(e->type == MEM_AVAILABLE){
            last_addr = e->addr;
            last_size = e->length;
        }
        cur_addr += e->size + sizeof(uintptr_t);
    }

    uint64_t total_size = last_addr + last_size;

    const uint64_t page_count = total_size / PAGE_SIZE;
    const uint32_t bitmap_size = div_ceil(page_count, 8);
    const uint32_t bitmap_page_count = div_ceil(bitmap_size, PAGE_SIZE);
    const uint32_t combined_page_count = kernel_page_count + bitmap_page_count;
    const char *combined_end_p = kernel_start_p + PAGE_SIZE * combined_page_count;

    uint32_t bitmapaddr = 0;
    if(kernel_entry != (void*)0 && kernel_entry->length / PAGE_SIZE >= combined_page_count){
        bitmapaddr = (uint32_t)kernel_start_p + combined_page_count * PAGE_SIZE;
    }
    else {
        return; // TODO: handle finding another section in memory
    }

    struct managed_memory *pmm = (struct managed_memory*)((char*)&early_pmm - 0xC0000000);

    pmm->bitmap = (uint8_t*)bitmapaddr;
    pmm->size = bitmap_size;
    pmm->next_index = 0;
    pmm->next_offset = 0;
    pmm->set_up = false;

    for(uint32_t i = 0; i < early_pmm.size; i++){
        early_pmm.bitmap[i] = (uint8_t)~0;
    }

    bool pmm_next_set = false;
    cur_addr = (uintptr_t)mmap_start;
    while(cur_addr < mmap_end){
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry*)cur_addr;
        if(e->type == MEM_AVAILABLE){
            uint32_t npages = e->length / PAGE_SIZE;
            for(uint32_t i = 0; i < npages; i++){
                char *page_addr = (char*)(e->addr + i * PAGE_SIZE);
                if(kernel_start_p <= page_addr && page_addr < combined_end_p)
                    continue;

                else if(page_addr < MB)
                    continue;

                uint32_t index = 0;
                uint32_t offset = 0;
                early_physaddr_to_offsets(page_addr, &index, &offset);
                pmm->bitmap[index] &= ~(1 << offset);

                if(pmm_next_set == false){
                    pmm->next_index = index;
                    pmm->next_offset = offset;
                    pmm_next_set = true;
                }
            }
        }
        cur_addr += e->size + sizeof(uintptr_t);
    }
}

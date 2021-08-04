#include <stdint.h>

#include <kernel/multiboot.h>
#include <kernel/tty.h>
#include <kernel/mem.h>

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

void iprint(uint64_t n){
    if( n > 9 ){
        uint64_t a = n / 10;

        n -= 10 * a;
        iprint(a);
    }
    terminal_putchar('0'+n);
}

void init_frame_allocator(struct multiboot_info *mbh_physaddr){
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
        int x = map_page((void*)(mmap_page_start + i * PAGE_SIZE), (void*)(0xC0000000 | ((pagen + i) << 12)), 0x1);
        if(x == MAP_PAGE_PHYSADDR_UNALIGNED){
            terminal_writestring("physical address unaligned\n");
        }
        else if(x == MAP_PAGE_VIRTUALADDR_UNALIGNED){
            terminal_writestring("virtual address unaligned\n");
        }
        else if(x == MAP_PAGE_MAPPING_ALREADY_PRESENT){
            terminal_writestring("mapping already present\n");
        }
        //cur_page += i * PAGE_SIZE;
    }

    struct multiboot_mmap_entry *mmap_start = 0xC0384000 | (mbh_vaddr->mmap_addr % PAGE_SIZE);
    //iprint(mmap_start[0].size);
    terminal_writestring("entry length: ");
    iprint((uint32_t)(mmap_start[0].length));
    terminal_putchar('\n');
    terminal_writestring("entry address: ");
    iprint((uint32_t)(mmap_start[0].addr));
    terminal_putchar('\n');

    terminal_writestring("amount of pages mapped: ");
    iprint(i);
    terminal_putchar('\n');

    uintptr_t cur_addr = mmap_start;
    uintptr_t mmap_end = cur_addr + mbh_vaddr->mmap_length;

    while(cur_addr < mmap_end){
        struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry*)cur_addr;
        terminal_writestring("memory area of size ");
        iprint(e->length);
        terminal_writestring(" starting at address ");
        iprint(e->addr);
        terminal_putchar('\n');
        cur_addr += e->size + sizeof(uintptr_t);
    }
}

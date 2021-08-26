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
        return NULL;
    }

    uint32_t page = *(uint32_t*)(0xFFC00000 | pdindex << 12 | ptindex << 2);
    if(!(page & 1)){
        return NULL;
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

    uint32_t *pdeptr = (uint32_t*)(0xFFFFF000 | pdindex << 2);
    if(!(*pdeptr & 1)){
        void *phys_page = get_page();
        *pdeptr = (uint32_t)phys_page | 0x103;
        flush_full_tlb();
    }

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
    of the PD which is recursively mapped at 0xFFC00000.

    If the heap "starts" at half of the space available to the kernel
    that is: PDE 768 + (1024 - 768) / 2 =
    (1024 - 768) / 2 = 128
    PDE = 768 + 128 = 896

    Then the space available to us will be 508M. This is 4M less than
    half of 1G, as 4M is taken up by the page directory being
    recursively mapped at the last page directory entry.
*/

struct heap_area {
    uint32_t next;
    uint32_t prev;
};

struct heap_area *const heap_start = (struct heap_area*)0xE0000000;
struct heap_area *const heap_end = (struct heap_area*)(0xFFC00000 - sizeof(struct heap_area) * 2);

/*
    We want to insure all addresses returned by kmalloc() and
    kcalloc() are 8-byte aligned to prevent UB for C in regards
    to dereferencing addresses which aren't aligned to the size
    of the type they are pointing to. The minimum required alignment
    for all C types is 8 bytes, for a 64-bit integer. We are
    ensuring 16-byte alignment for memory area start because
    `struct heap_area` is 8 bytes in size, meaning the area it is
    the head of will always be 8-byte aligned. It also allows
    previously allocated memory areas a small amount of expansion.

    Since we want all addresses      +--------+-----+----------+
    returned by kmalloc() to be      | member | bit | meaning  |
    8-byte aligned, and every        +--------+-----+----------+
    `struct heap_area` to be         | next   | 0   | used y/n |
    16-byte aligned, the lowest      +--------+-----+----------+
    4 bits of both the `next`        | next   | 1   | Epsilon  |
    and `prev` member of             +--------+-----+----------+
    `struct heap_area` will be       | next   | 2   | None     |
    clear. We will reserve           +--------+-----+----------+
    these bits and use them          | next   | 3   | None     |
    for additional information.      +--------+-----+----------+
                                     | prev   | 0   | None     |
    Not all of the reserved          +--------+-----+----------+
    bits have an assigned            | prev   | 1   | None     |
    meaning as of yet. They          +--------+-----+----------+
    are still reserved for           | prev   | 2   | None     |
    future use, so as to             +--------+-----+----------+
    allow future                     | prev   | 3   | None     |
    expandability and to             +--------+-----+----------+
    ensure 16-byte alignment.        
*/

static inline struct heap_area *get_next_address(struct heap_area *area){
    return (struct heap_area*)(area->next & ~0b1111);
}

static inline void set_next_address(struct heap_area *area, struct heap_area *next){
    uint32_t n = (uint32_t)next & ~0b1111;
    uint32_t flags = area->next & 0b1111;
    area->next = n | flags;
}

static inline struct heap_area *get_prev_address(struct heap_area *area){
    return (struct heap_area*)(area->prev & ~0b1111);
}

static inline void set_prev_address(struct heap_area *area, struct heap_area *prev){
    uint32_t n = (uint32_t)prev & ~0b1111;
    uint32_t flags = area->prev & 0b1111;
    area->prev = n | flags;
}

static inline bool get_area_used(struct heap_area *area){
    return (bool)(area->next & 1U);
}

static inline void set_area_used(struct heap_area *area, bool used){
    if(used == true)
        area->next |= 1;
    else
        area->next &= ~1UL;
}

static inline bool get_area_epsilon(struct heap_area *area){
    return (bool)((area->next >> 1) & 1U);
}

static inline void set_area_epsilon(struct heap_area *area, bool ep){
    if(ep == true)
        area->next |= 1UL << 1;
    else
        area->next &= ~(1UL << 1);
}

static inline size_t get_area_size(struct heap_area *area){
    struct heap_area *next = get_next_address(area);
    if(get_area_epsilon(area) || next == NULL)
        return 0;

    return (uint32_t)next - (uint32_t)area - sizeof(struct heap_area);
}

static void merge_free_areas(struct heap_area *area){
    struct heap_area *prev = get_prev_address(area);
    struct heap_area *next = get_next_address(area);

    bool merge_towards_root = prev != NULL && get_area_used(prev) == false;
    bool merge_towards_tail = next != NULL && get_area_used(next) == false;

    if(merge_towards_root && merge_towards_tail){
        struct heap_area *na = get_next_address(next);
        set_next_address(prev, na);
        set_prev_address(na, prev);
    }
    else if(merge_towards_tail){
        set_next_address(area, get_next_address(next));
        set_prev_address(next, area);
    }
    else if(merge_towards_root){
        set_prev_address(next, prev);
        set_next_address(prev, next);
    }
}


//TODO: check if there is enough physical memory to spare,
// so the entire kernel heap can be mapped at init time.
void init_heap(){
    map_page(get_page(), heap_start, 0x103);
    map_page(get_page(), (void*)((uint32_t)heap_end & ~0b111111111111), 0x103);
    set_next_address(heap_start, heap_end);
    set_prev_address(heap_start, NULL);
    set_area_epsilon(heap_start, false);
    set_area_used(heap_start, false);

    set_next_address(heap_end, NULL);
    set_prev_address(heap_end, heap_start);
    set_area_epsilon(heap_end, true);

    terminal_writestring("kernel heap initialized with heap size: ");
    iprint(get_area_size(heap_start));
    terminal_putchar('\n');
}


//TODO: maybe enforce kmalloc() return to be 16-byte aligned?
void *kmalloc(size_t size){
    if (size == 0)
        return NULL;

    //Ensure there is space for the padding we require to
    // have memory areas start align with 16 bytes.
    size = div_ceil(size + sizeof(struct heap_area), 16) * 16;

    //Initialize best_size to 1G (twice the size of the heap)
    // so the best-fit algorithm always replaces the initial
    // value in the loop; this saves us one if-statement. For
    // this, any value larger than the heap would suffice.
    size_t best_size = 1073741824;
    struct heap_area *best_fit = NULL;

    for(struct heap_area *a = heap_start; get_area_epsilon(a) == false; a = get_next_address(a)){
        //Look for area with smallest difference between wanted size
        // and area size.
        size_t area_size = 0;
        if(get_area_used(a) == true)
            continue;
        else if((area_size = get_area_size(a)) > size && best_size > area_size - size){
            best_size = area_size;
            best_fit = a;
        }
    }

    //Insert new area head at 16-byte alligned address if
    // free space left over in current area is of considerable
    // magnitude.

    //We only check for page mappings when we insert a new area
    // head, because if the next area head is sufficiently close
    // as to forego adding a new one, the pages up until then
    // have already been mapped by previous allocations, or we
    // are at the end of the heap.
    if(best_size - size > sizeof(struct heap_area) * 2){
        struct heap_area *new = (struct heap_area*)((uint32_t)best_fit + size);

        char *start_page = (char*)((uint32_t)best_fit & ~0b111111111111);
        char *end_page = (char*)((uint32_t)new & ~0b111111111111);

        //TODO: add VMM (at least for kernel memory) so checking if
        // the page is already mapped is less computationally expensive.
        for(char *p = start_page; p <= end_page; p += PAGE_SIZE){
            if(get_physaddr(p) == NULL){
                map_page(get_page(), p, 0x103);
            }
        }

        new->prev = 0;
        new->next = 0;
        set_next_address(new, get_next_address(best_fit));
        set_prev_address(new, best_fit);
        set_next_address(best_fit, new);
    }

    set_area_used(best_fit, true);

    return (void*)(best_fit+1);
}

void *kcalloc(size_t nmemb, size_t size){
    size_t total = nmemb * size;
    if(nmemb == 0 || size == 0)
        return NULL;
    else if(size != total / nmemb)
        return NULL; //return NULL on overflow

    void *km = kmalloc(total);
    if(km == NULL)
        return km;

    return memset(km, 0, total);
}

//TODO: check if next area is unused and
// expand current area?
void *krealloc(void *ptr, size_t size){
    if(size == 0)
        return NULL;
    else if(ptr == NULL)
        return kmalloc(size);

    struct heap_area *a = (struct heap_area*)ptr - 1;
    size_t area_size = get_area_size(a);
    if(area_size >= size){
        return ptr;
    }

    void *new = kmalloc(size);
    if(new == NULL)
        return NULL;

    memcpy(new, ptr, area_size);
    kfree(ptr);
    return new;
}


//TODO: implement kpalloc()/kpagealloc()
void *kpagealloc(size_t n){

}

void kfree(void *ptr){
    if(ptr == NULL) return;
    
    struct heap_area *a = (struct heap_area*)ptr - 1;

    if(get_area_used(a) == false) return;

    set_area_used(a, false);
    merge_free_areas(a);
}

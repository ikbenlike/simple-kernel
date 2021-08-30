#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <kernel/klimits.h>
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
        void *phys_page = get_page("map_page()\n");
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

/*TODO: fix this function because its a whole mess
and it returns the same value multiple times, also
seems to cause page fault.
void *get_page(){
    pmm.bitmap[pmm.next_index] |= 1 << pmm.next_offset;
    void *r = offsets_to_physaddr(pmm.next_index, pmm.next_offset);

    terminal_writestring("index at start: ");
    iprint(pmm.next_index);
    terminal_putchar('\n');

    //size_t id = 0;
    //size_t of = 0;

    terminal_writestring("pmm value: ");
    iprint(pmm.bitmap[pmm.next_index]);
    terminal_putchar('\n');

    if(pmm.bitmap[pmm.next_index] == (uint8_t)~0U){        
        for(uint32_t i = pmm.next_index; i < pmm.size; i++){
        //for(uint32_t i = 0; i < pmm.size; i++){
            if(pmm.bitmap[i] < (uint8_t)~0){
                uint8_t j = first_zero_in_byte(pmm.bitmap[i]);
                pmm.next_offset = i;
                pmm.next_index = j;
                //of = i;
                //id = j;
                break;
            }
        }
    }
    else {
        pmm.next_offset = first_zero_in_byte(pmm.bitmap[pmm.next_index]);
    }

    terminal_writestring("index at end: ");
    iprint(pmm.next_index);
    terminal_putchar('\n');

    //pmm.bitmap[id] |= 1 << of;
    //return offsets_to_physaddr(id, of);
    return r;
}*/

void *get_page(char *f){
    terminal_writestring(f);
    terminal_writestring("pmm.size: ");
    iprint(pmm.size);
    terminal_putchar('\n');

    terminal_writestring("pmm.free_pages: ");
    iprint(pmm.free_pages);
    terminal_putchar('\n');

    void *page = NULL;

    uint32_t i = 0;
    uint8_t o = 0;

    for(i = 0; i < pmm.size; i++){
        if(pmm.bitmap[i] != 0xFF){
            terminal_writestring("pmm.bitmap[i] != 0xFF\n");
            o = first_zero_in_byte(pmm.bitmap[i]);
            page = offsets_to_physaddr(i, o);
            pmm.bitmap[i] |= 1 << o;
            break;
        }
    }

    terminal_writestring("index, offset: ");
    iprint(i);
    terminal_writestring(", ");
    iprint(o);
    terminal_putchar('\n');
    //while(1){};

    if(page == NULL){
    }

    return page;
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
struct heap_area *const heap_end = (struct heap_area*)(0xFFC00000 - sizeof(struct heap_area));

/*
    We want to insure all addresses returned by kmalloc() and
    kcalloc() are 8-byte aligned to prevent UB for C in regards
    to dereferencing addresses which aren't aligned to the size
    of the type they are pointing to. The minimum required alignment
    for all C types is 8 bytes, for a 64-bit integer. We are
    ensuring 8-byte alignment for memory area start because
    `struct heap_area` is 8 bytes in size, meaning the area it is
    the head of will also always be 8-byte aligned.

    Since we want all addresses      +--------+-----+----------+
    returned by kmalloc() to be      | member | bit | meaning  |
    8-byte aligned, and every        +--------+-----+----------+
    `struct heap_area` to be         | next   | 0   | used y/n |
    8-byte aligned, the lowest       +--------+-----+----------+
    3 bits of both the `next`        | next   | 1   | Epsilon  |
    and `prev` member of             +--------+-----+----------+
    `struct heap_area` will be       | next   | 2   | None     |
    clear. We will reserve           +--------+-----+----------+
    these bits and use them          | prev   | 0   | None     |
    for additional information.      +--------+-----+----------+
                                     | prev   | 1   | None     |
    Not all of the reserved          +--------+-----+----------+
    bits have an assigned            | prev   | 2   | None     |
    meaning as of yet. They          +--------+-----+----------+
    are still reserved for
    future use, so as to
    allow future
    expandability and to
    ensure 16-byte alignment.        
*/

static inline struct heap_area *get_next_address(struct heap_area *area){
    return (struct heap_area*)(area->next & ~0b111);
}

static inline void set_next_address(struct heap_area *area, struct heap_area *next){
    uint32_t n = (uint32_t)next & ~0b111;
    uint32_t flags = area->next & 0b111;
    area->next = n | flags;
}

static inline struct heap_area *get_prev_address(struct heap_area *area){
    return (struct heap_area*)(area->prev & ~0b111);
}

static inline void set_prev_address(struct heap_area *area, struct heap_area *prev){
    uint32_t n = (uint32_t)prev & ~0b111;
    uint32_t flags = area->prev & 0b111;
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
    if(area == NULL){
        terminal_writestring("epsilon(area) == NULL\n");
        while(1);
    }
    //terminal_writestring("get_area_epsilon()\n");
    bool r = (bool)((area->next >> 1) & 1U);
    //if(r == true) terminal_writestring("epsilon == true\n");
    //else terminal_writestring("epsilon == false\n");
    return r;

    //return (bool)((area->next >> 1) & 1U);
}

static inline void set_area_epsilon(struct heap_area *area, bool ep){
    if(ep == true)
        area->next |= 1UL << 1;
    else
        area->next &= ~(1UL << 1);
}

static inline size_t get_area_size(struct heap_area *area){
    struct heap_area *next = get_next_address(area);
    //terminal_writestring("get_area_size()\n");
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
    static bool ran = false;
    if(ran == true) return;

    map_page(get_page("init_heap()\n"), heap_start, 0x103);
    map_page(get_page("init_heap()\n"), (void*)((uint32_t)heap_end & ~0b111111111111), 0x103);
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

    ran = true;
}

//TODO: maybe enforce kmalloc() return to be 16-byte aligned?
void *kmalloc(size_t size){
    if (size == 0)
        return NULL;

    //Ensure there is space for the padding we require to
    // have memory areas start align with 16 bytes.
    size = div_ceil(size + sizeof(struct heap_area), 8) * 8;
    //TODO: see if `sizeof(struct heap_area)` is really needed here

    //Initialize best_size to 1G (twice the size of the heap)
    // so the best-fit algorithm always replaces the initial
    // value in the loop; this saves us one if-statement. For
    // this, any value larger than the heap would suffice.
    size_t best_size = 1073741824;
    struct heap_area *best_fit = NULL;

    for(struct heap_area *a = heap_start; get_area_epsilon(a) == false; a = get_next_address(a)){
        //Look for area with smallest difference between wanted size
        // and area size.

        /*terminal_writestring("a->prev: ");
        iprint((uint32_t)(a->prev));
        terminal_putchar('\n');
        terminal_writestring("a->next: ");
        iprint((uint32_t)(a->next));
        terminal_putchar('\n');*/

        //terminal_writestring("kmalloc loop\n");
        size_t area_size = 0;
        if(get_area_used(a) == true){
            //terminal_writestring("area used!\n");
            continue;
        }
        else if((area_size = get_area_size(a)) > size && best_size - size > area_size - size){
            //terminal_writestring("area fit\n");
            best_size = area_size;
            best_fit = a;
        }
        //terminal_writestring("end of loop\n");
    }

    //terminal_writestring("after loop\n");

    if(best_fit == NULL) return NULL;

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
                map_page(get_page("kmalloc()\n"), p, 0x103);
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

struct heap_area *new_tail = NULL;

char *check_new_tail(){
    if(new_tail == NULL) return NULL;

    terminal_writestring("new_tail->prev: ");
    iprint((uint32_t)get_prev_address(new_tail));
    terminal_putchar('\n');
    terminal_writestring("new_tail->next: ");
    iprint((uint32_t)get_next_address(new_tail));
    terminal_putchar('\n');

    return (char*)new_tail;
}

//TODO: implement kpalloc()/kpagealloc()
void *kpagealloc(size_t n){
    if(n == 0)
        return NULL;

    size_t size = n * PAGE_SIZE;
    size_t best_size = GB;
    struct heap_area *best_fit = NULL;

    for(struct heap_area *a = heap_start; get_area_epsilon(a) == false; a = get_next_address(a)){
        size_t area_size = 0;
        if(get_area_used(a) == true)
            continue;
        else if((area_size = get_area_size(a)) > size && best_size - size > area_size - size){
            uint32_t next_page = ((uint32_t)a & ~0b111111111111) + PAGE_SIZE;

            size_t full_size = size + (next_page - (uint32_t)a) + sizeof(struct heap_area);
            if(next_page - sizeof(struct heap_area) != (uint32_t)a)
                full_size += sizeof(struct heap_area);

            if(area_size > full_size && best_size - full_size > area_size - full_size){
                best_size = area_size;
                best_fit = a;
            }
        }
    }

    if(best_fit == NULL) return NULL;

    /*if(best_fit == heap_start && get_next_address(heap_start) == heap_end){
        terminal_writestring("it's heap_start\n");
    }*/

    struct heap_area *ret = best_fit;
    uint32_t next_page = ((uint32_t)best_fit & ~0b111111111111) + PAGE_SIZE;
    size_t full_size = size + (next_page - (uint32_t)best_fit) + sizeof(struct heap_area);

    //Insert a new head right before the first page that is
    // allocated if the previous free area is large enough
    // to permit this.
    if(next_page - sizeof(struct heap_area) != (uint32_t)ret){
        struct heap_area *next = get_next_address(ret);
        ret = (struct heap_area*)(next_page - sizeof(struct heap_area));
        set_prev_address(ret, best_fit);
        set_next_address(ret, next);
        set_prev_address(next, ret);
        set_next_address(best_fit, ret);

        if(get_prev_address(ret) == heap_start && get_next_address(ret) == heap_end){
            terminal_writestring("new start added!\n");
        }
    }

    if(best_size - full_size > sizeof(struct heap_area) * 2){
        //Insert a head at the end of the allocated area if
        // the free area after the allocated area is large
        // enough to permit this.
        struct heap_area *new = (struct heap_area*)((uint32_t)ret + size + sizeof(struct heap_area));

        /*if((uint32_t)new % PAGE_SIZE == 0){
            terminal_writestring("new end element is page-aligned!\n");
        }

        if((uint32_t)new - (uint32_t)ret - sizeof(struct heap_area) == size){
            terminal_writestring("size is correct!\n");
        }

        uint32_t area_start = (uint32_t)(ret + 1);
        if((uint32_t)new >= area_start + size){
            terminal_writestring("new is outside allocation\n");
        }

        if(ret > heap_start){
            terminal_writestring("area is in heap!\n");
        }*/

        //char *start_page = (char*)((uint32_t)ret & ~0b111111111111);
        //char *end_page = (char*)((uint32_t)new & ~0b111111111111);

        char *start_page = (char*)next_page;
        char *end_page = (char*)new;

        //Check if all the allocated pages are mapped.
        //TODO: see kmalloc() todo
        size_t i = 0;
        void *gps[80] = {0};
        for(char *p = start_page; p <= end_page; p += PAGE_SIZE){
            if(get_physaddr(p) == NULL){
                void *gp = get_page("kpagealloc()\n");
                map_page(gp, p, 0x103);
                iprint((uint32_t)gp);
                terminal_putchar('\n');
                for(size_t x = 0; x < i; x++){
                    if(gp == gps[x]){
                        terminal_writestring("page already present!\n");
                    }
                }
                gps[i++] = gp;

                terminal_writestring("page mapped!\n");
            }
        }

        //while(1){};

        new->prev = 0;
        new->next = 0;
        set_next_address(new, get_next_address(ret));
        set_prev_address(new, ret);
        set_next_address(ret, new);

        terminal_writestring("allocated area size: ");
        iprint(get_area_size(ret));
        terminal_putchar('\n');

        terminal_writestring("requested size: ");
        iprint(size);
        terminal_putchar('\n');

        /*if(get_prev_address(new) == ret){
            terminal_writestring("new->prev == ret!\n");
        }

        if(get_prev_address(get_prev_address(new)) == heap_start){
            terminal_writestring("prev value is correct!\n");
        }

        if(get_next_address(new) == heap_end){
            terminal_writestring("next value is correct!\n");
        }*/

        /*if(get_prev_address(get_prev_address(new)) == heap_start && get_next_address(new) == heap_end){
            terminal_writestring("next/prev values are correct!\n");
        }

        if(get_area_size(new)){
            terminal_writestring("tail area has size!\n");
        }*/

        new_tail = new;
    }

    set_area_used(ret, true);

    return (void*)(ret + 1);
}

void kfree(void *ptr){
    if(ptr == NULL) return;
    
    struct heap_area *a = (struct heap_area*)ptr - 1;

    if(get_area_used(a) == false) return;

    set_area_used(a, false);
    merge_free_areas(a);
}

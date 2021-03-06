#ifndef __ARCH_I386_MEM_H__

    #define __ARCH_I386_MEM_H__


    #ifdef __cplusplus
        extern "C" {
    #endif

    #include <stdint.h>

    enum PageErrors {
        GET_PHYSADDR_PD_INVALID = 1,
        GET_PHYSADDR_PT_INVALID = 2,
        MAP_PAGE_PHYSADDR_UNALIGNED = 3,
        MAP_PAGE_VIRTUALADDR_UNALIGNED = 4,
        MAP_PAGE_MAPPING_ALREADY_PRESENT = 5,
    };

    struct managed_memory {
        uint8_t *bitmap;
        uint32_t size;
        uint32_t next_index;
        uint8_t next_offset;
        uint32_t free_pages;
        bool set_up;
    };

    void flush_full_tlb();
    void *get_physaddr(void *virtualaddr);
    int map_page(void *physaddr, void *virtualaddr, uint16_t flags);
    int unmap_page(void *virtualaddr);
    bool get_page_state(void *physaddr);
    void *get_page();
    void free_page(void *physaddr);
    void late_pmm_init(struct managed_memory p);

    void init_heap();

    void *kmalloc(size_t size);
    void *kcalloc(size_t nmemb, size_t size);
    void *krealloc(void *ptr, size_t size);
    void *kpagealloc(size_t n);
    void kfree(void *ptr);

    #ifdef __cplusplus
        }
    #endif

#endif

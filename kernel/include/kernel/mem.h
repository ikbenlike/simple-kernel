#ifndef __ARCH_I386_MEM_H__

    #define __KERNEL_MEM_H__


    #ifdef __cplusplus
        extern "C" {
    #endif

    #define PAGE_SIZE 4096

    enum PageErrors {
        GET_PHYSADDR_PD_INVALID = 1,
        GET_PHYSADDR_PT_INVALID = 2,
        MAP_PAGE_PHYSADDR_UNALIGNED = 3,
        MAP_PAGE_VIRTUALADDR_UNALIGNED = 4,
        MAP_PAGE_MAPPING_ALREADY_PRESENT = 5,
    };

    void flush_full_tlb();
    void *get_physaddr(void *virtualaddr);
    int map_page(void *physaddr, void *virtualaddr, uint16_t flags);
    void init_frame_allocator(struct multiboot_info *mbh_physaddr);

    #ifdef __cplusplus
        }
    #endif

#endif

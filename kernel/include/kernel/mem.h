#ifndef __ARCH_I386_MEM_H__

    #define __ARCH_I386_MEM_H__


    #ifdef __cplusplus
        extern "C" {
    #endif

    void *get_physaddr(void *virtualaddr);

    #ifdef __cplusplus
        }
    #endif

#endif

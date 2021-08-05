#ifndef __ARCHI386_UTIL_H__
    
    #define __ARCHI386_UTIL_H__

    #ifdef __cplusplus
        extern "C" {
    #endif

    #include <stdint.h>

    inline size_t div_ceil(size_t a, size_t b){
        return (a + b - 1) / b;
    }

    #ifdef __cplusplus
        }
    #endif

#endif

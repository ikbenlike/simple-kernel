#ifndef __STRING_H_
    #define __STRING_H_
    #include <stddef.h>

    #ifdef __cplusplus
        extern "C" {
    #endif

    size_t strlen(const char *str);
    void *memset(void *bufptr, int value, size_t size);
    void *memmove(void* dstptr, const void* srcptr, size_t size);
    void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
    int memcmp(const void *aptr, const void *bptr, size_t size);

    char *strcpy(char *restrict dest, const char *src);

    #ifdef __cplusplus
        }
    #endif
#endif

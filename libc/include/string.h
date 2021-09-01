#ifndef __STRING_H_
    #define __STRING_H_
    #include <stddef.h>

    #ifdef __cplusplus
        extern "C" {
    #endif

    int memcmp(const void *aptr, const void *bptr, size_t size);
    void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
    void *memmove(void* dstptr, const void* srcptr, size_t size);
    void *memset(void *bufptr, int value, size_t size);

    int strcmp(const char *s1, const char *s2);
    char *strcpy(char *restrict dest, const char *src);
    size_t strlen(const char *str);

    #ifdef __cplusplus
        }
    #endif
#endif

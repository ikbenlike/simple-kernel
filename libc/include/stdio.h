#ifndef __STDIO_H_
    #define __STDIO_H_

    #include <sys/cdefs.h>

    #define EOF (-1)

    #ifdef __cplusplus
        extern "C" {
    #endif

        int printf(const char *__restrict, ...);
        int puts(const char *str);
        int putchar(int ca);

    #ifdef __cplusplus
        }
    #endif

#endif 

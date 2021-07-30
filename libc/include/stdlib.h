#ifndef __STDLIB_H_
    #define __STDLIB_H_ 

    #include <sys/cdefs.h>
    #include <stdio.h>

    #ifdef __cplusplus
        extern "C" {
    #endif

    __attribute__((__noreturn__))
    void abort(void);

    #ifdef __cplusplus
        }
    #endif
#endif

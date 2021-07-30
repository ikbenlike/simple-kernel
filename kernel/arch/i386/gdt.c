#include <string.h>

#include "gdt.h"

#if defined(__linux__) || !defined(__i386__)
#error "An i386 cross compiler is necessary to compile this module."
#endif

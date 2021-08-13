#define WRAPPER_DEF(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        pushal; \
        cld; \
        call f; \
        popal; \
        iret;

.section .text
WRAPPER_DEF(0, interrupt_handler)

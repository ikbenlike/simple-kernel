#define WRAPPER_DEF_NO_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        cli; \
        pushal; \
        pushl $0; \
        cld; \
        call f; \
        popal; \
        sti; \
        iret;

#define WRAPPER_DEF_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        cli; \
        pushal; \
        cld; \
        call f; \
        popal; \
        sti; \
        iret;

.section .text
WRAPPER_DEF_NO_ERR(0, interrupt_handler)

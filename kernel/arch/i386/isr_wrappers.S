#define WRAPPER_DEF_NO_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        cli; \
        pushl $0; \
        pushal; \
        cld; \
        call f; \
        popal; \
        addl $4, %esp; \
        sti; \
        iret; \

#define WRAPPER_DEF_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        cli; \
        pushal; \
        cld; \
        call f; \
        popal; \
        addl $4, %esp; \
        sti; \
        iret; \

.section .text
WRAPPER_DEF_NO_ERR(0, interrupt_handler)

WRAPPER_DEF_NO_ERR(reserved, reserved_interrupt)

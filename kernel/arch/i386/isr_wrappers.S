#define WRAPPER_DEF_NO_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        pushl $0; \
        pushal; \
        cld; \
        call f; \
        popal; \
        addl $4, %esp; \
        iret; \

#define WRAPPER_DEF_ERR(x, f) \
    .global isr_wrapper_##x; \
    .type isr_wrapper_##x, @function; \
    isr_wrapper_##x: \
        pushal; \
        cld; \
        call f; \
        popal; \
        addl $4, %esp; \
        iret; \

.section .text
WRAPPER_DEF_NO_ERR(0, zero_division_handler)
WRAPPER_DEF_NO_ERR(1, debug_exception_handler)
WRAPPER_DEF_NO_ERR(2, nmi_interrupt_handler)
WRAPPER_DEF_NO_ERR(3, breakpoint_exception_handler)
WRAPPER_DEF_NO_ERR(4, overflow_exception_handler)
WRAPPER_DEF_NO_ERR(5, bound_range_exception_handler)
WRAPPER_DEF_NO_ERR(6, invalid_opcode_handler)
WRAPPER_DEF_NO_ERR(7, not_available_exception_handler)
WRAPPER_DEF_ERR(8, double_fault_handler)
WRAPPER_DEF_NO_ERR(9, coprocessor_overrun_handler)
WRAPPER_DEF_ERR(10, invalid_tss_handler)
WRAPPER_DEF_ERR(11, segment_not_present_handler)
WRAPPER_DEF_ERR(12, stack_fault_handler)
WRAPPER_DEF_ERR(13, general_protection_handler)
WRAPPER_DEF_ERR(14, page_fault_handler)
WRAPPER_DEF_NO_ERR(16, floating_point_fault_handler)
WRAPPER_DEF_ERR(17, alignment_check_handler)
WRAPPER_DEF_NO_ERR(18, machine_check_handler)
WRAPPER_DEF_NO_ERR(19, simd_exception_handler)
WRAPPER_DEF_NO_ERR(20, virtualization_exception_handler)
WRAPPER_DEF_ERR(21, control_protection_handler)

WRAPPER_DEF_NO_ERR(reserved, reserved_interrupt)

WRAPPER_DEF_NO_ERR(32, pit_handler)
WRAPPER_DEF_NO_ERR(33, keyboard_handler)

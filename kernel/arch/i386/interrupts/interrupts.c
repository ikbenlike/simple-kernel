#include <stdint.h>

#include <kernel/tty.h>
#include <kernel/idt.h>
#include <kernel/pic.h>

extern void iprint(uint64_t);

void zero_division_handler(struct stackframe sf){
    if((sf.cs & 0b11) == 3){
        //TODO when userspace gets added: terminate offending program
    }
    else {
        terminal_writestring("FATAL ERROR: division-by-zero occurred in kernel-space.");
        while(1){};
    }
}

void debug_exception_handler(struct stackframe sf){

}

void nmi_interrupt_handler(struct stackframe sf){

}

void breakpoint_exception_handler(struct stackframe sf){

}

void overflow_exception_handler(struct stackframe sf){

}

void bound_range_exception_handler(struct stackframe sf){

}

void invalid_opcode_handler(struct stackframe sf){
    if((sf.cs & 0b11) == 3){
        //TODO terminate program
    }
    else {
        terminal_writestring("FATAL ERROR: the kernel encountered an invalid opcode during execution.");
        while(1){};
    }
}

void not_available_exception_handler(struct stackframe sf){

}

void double_fault_handler(struct stackframe sf){

}

void coprocessor_overrun_handler(struct stackframe sf){
    if((sf.cs & 0b11) == 3){
        //TODO terminate program
    }
    else {
        terminal_writestring("FATAL ERROR: coprocessor segment overrun in kernel-space.");
        while(1){};
    }
}

void invalid_tss_handler(struct stackframe sf){

}

void segment_not_present_handler(struct stackframe sf){

}

void stack_fault_handler(struct stackframe sf){

}

void general_protection_handler(struct stackframe sf){

}

void page_fault_handler(struct stackframe sf){
    terminal_writestring("FATAL ERROR: page fault!\n");
    terminal_writestring("  error code: ");
    iprint(sf.err);
    while(1){};
}

void floating_point_fault_handler(struct stackframe sf){

}

void alignment_check_handler(struct stackframe sf){

}

void machine_check_handler(struct stackframe sf){

}

void simd_exception_handler(struct stackframe sf){

}

void virtualization_exception_handler(struct stackframe sf){

}

void control_protection_handler(struct stackframe sf){

}

void reserved_interrupt(struct stackframe sf){
    if((sf.cs & 0b11) == 3){
        //TODO terminate program
    }
    else {
        terminal_writestring("FATAL ERROR: this isn't supposed to happen! Reserved interrupt fired.");
        while(1){};
    }

}

void pit_handler(struct stackframe sf){
    pic_send_eoi(0);
}

void keyboard_handler(struct stackframe sf){
    terminal_writestring("pressed");
    uint8_t code = inb(0x60);
    pic_send_eoi(1);
}

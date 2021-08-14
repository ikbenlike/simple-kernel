#ifndef __ARCH_I386_INTERRUPTS_H__

    #define __ARCH_I386_INTERRUPTS_H__

    void interrupt_handler(struct stackframe);
    void reserved_interrupt(struct stackframe fs);

#endif

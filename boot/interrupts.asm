BITS 32

section .text

global irq0_stub
extern timer_interrupt_handler

irq0_stub:
    ; Save the general-purpose registers.
    pusha

    ; C expects the direction flag to be clear.
    cld

    ; Run the C timer handler.
    call timer_interrupt_handler

    ; Restore the registers.
    popa

    ; Return from the interrupt.
    iretd
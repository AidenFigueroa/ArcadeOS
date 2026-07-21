#include "interrupts.h"
#include "io.h"

#define IDT_SIZE 256

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC_EOI      0x20

/*
 * Timer interrupt assembly entry point.
 */
extern void irq0_stub(void);


/*
 * One entry inside the Interrupt Descriptor Table.
 */
struct idt_entry
{
    unsigned short handler_low;
    unsigned short code_selector;
    unsigned char reserved;
    unsigned char attributes;
    unsigned short handler_high;
} __attribute__((packed));


/*
 * Structure passed to the LIDT CPU instruction.
 */
struct idt_descriptor
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));


static struct idt_entry idt[IDT_SIZE];
static struct idt_descriptor idt_pointer;


/*
 * Creates one IDT entry.
 */
static void idt_set_gate(
    unsigned char vector,
    unsigned int handler,
    unsigned short code_selector,
    unsigned char attributes
)
{
    idt[vector].handler_low =
        handler & 0xFFFF;

    idt[vector].code_selector =
        code_selector;

    idt[vector].reserved =
        0;

    idt[vector].attributes =
        attributes;

    idt[vector].handler_high =
        (handler >> 16) & 0xFFFF;
}


/*
 * Returns the currently active code-segment selector.
 */
static unsigned short get_code_selector(void)
{
    unsigned short selector;

    __asm__ volatile (
        "mov %%cs, %0"
        : "=r"(selector)
    );

    return selector;
}


/*
 * Moves hardware IRQ interrupts away from CPU exception
 * vectors 0 through 31.
 *
 * Master PIC begins at vector 32.
 * Slave PIC begins at vector 40.
 */
static void pic_remap(void)
{
    outb(PIC1_COMMAND, 0x11);
    io_wait();

    outb(PIC2_COMMAND, 0x11);
    io_wait();

    /*
     * IRQ 0 through IRQ 7 become vectors 32 through 39.
     */
    outb(PIC1_DATA, 0x20);
    io_wait();

    /*
     * IRQ 8 through IRQ 15 become vectors 40 through 47.
     */
    outb(PIC2_DATA, 0x28);
    io_wait();

    /*
     * Tell the master PIC that the slave is connected
     * through IRQ 2.
     */
    outb(PIC1_DATA, 0x04);
    io_wait();

    /*
     * Tell the slave PIC its cascade identity.
     */
    outb(PIC2_DATA, 0x02);
    io_wait();

    /*
     * Put both PICs into 8086 mode.
     */
    outb(PIC1_DATA, 0x01);
    io_wait();

    outb(PIC2_DATA, 0x01);
    io_wait();

    /*
     * Enable only IRQ 0, the timer.
     *
     * 0 bit = enabled
     * 1 bit = disabled
     */
    outb(PIC1_DATA, 0xFE);
    outb(PIC2_DATA, 0xFF);
}


/*
 * Creates and loads the Interrupt Descriptor Table.
 */
void interrupts_initialize(void)
{
    interrupts_disable();

    /*
     * Clear every IDT entry.
     */
    for (int index = 0; index < IDT_SIZE; index++)
    {
        idt[index].handler_low = 0;
        idt[index].code_selector = 0;
        idt[index].reserved = 0;
        idt[index].attributes = 0;
        idt[index].handler_high = 0;
    }

    /*
     * IRQ 0 is mapped to interrupt vector 32.
     *
     * 0x8E means:
     *   present
     *   kernel privilege
     *   32-bit interrupt gate
     */
    idt_set_gate(
        32,
        (unsigned int)irq0_stub,
        get_code_selector(),
        0x8E
    );

    idt_pointer.limit =
        sizeof(idt) - 1;

    idt_pointer.base =
        (unsigned int)&idt;

    /*
     * Load the IDT into the CPU.
     */
    __asm__ volatile (
        "lidt %0"
        :
        : "m"(idt_pointer)
    );

    pic_remap();
}


/*
 * Enables hardware interrupts.
 */
void interrupts_enable(void)
{
    __asm__ volatile ("sti");
}


/*
 * Disables hardware interrupts.
 */
void interrupts_disable(void)
{
    __asm__ volatile ("cli");
}


/*
 * Tells the PIC that an interrupt has been handled.
 */
void interrupts_send_eoi(unsigned char irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }

    outb(PIC1_COMMAND, PIC_EOI);
}
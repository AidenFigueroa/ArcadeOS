#include "timer.h"
#include "interrupts.h"
#include "io.h"

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

#define PIT_BASE_FREQUENCY 1193182

static volatile unsigned int timer_ticks = 0;


/*
 * Configures the Programmable Interval Timer.
 */
void timer_initialize(unsigned int frequency)
{
    if (frequency == 0)
    {
        frequency = 100;
    }

    unsigned int divisor =
        PIT_BASE_FREQUENCY / frequency;

    /*
     * The PIT divisor must fit inside 16 bits.
     */
    if (divisor > 65535)
    {
        divisor = 65535;
    }

    if (divisor < 1)
    {
        divisor = 1;
    }

    /*
     * 0x36 configures channel 0:
     *
     *   low byte followed by high byte
     *   square-wave mode
     *   binary counting
     */
    outb(PIT_COMMAND_PORT, 0x36);

    unsigned char low_byte =
        divisor & 0xFF;

    unsigned char high_byte =
        (divisor >> 8) & 0xFF;

    outb(PIT_CHANNEL0_PORT, low_byte);
    outb(PIT_CHANNEL0_PORT, high_byte);
}


/*
 * Called by irq0_stub every time the timer interrupts.
 */
void timer_interrupt_handler(void)
{
    timer_ticks++;

    /*
     * Tell the PIC that IRQ 0 has been handled.
     */
    interrupts_send_eoi(0);
}


/*
 * Returns the number of timer interrupts received.
 */
unsigned int timer_get_ticks(void)
{
    return timer_ticks;
}
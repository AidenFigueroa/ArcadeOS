#ifndef IO_H
#define IO_H

/*
 * Reads one byte from a hardware I/O port.
 */
static inline unsigned char inb(unsigned short port)
{
    unsigned char value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


/*
 * Writes one byte to a hardware I/O port.
 */
static inline void outb(
    unsigned short port,
    unsigned char value
)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


/*
 * Adds a tiny delay for older hardware controllers.
 */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

#endif
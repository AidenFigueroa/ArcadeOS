#include "memory.h"

#define HEAP_SIZE (1024 * 1024)
#define MEMORY_ALIGNMENT 16

/*
 * Reserve 1 MiB for dynamic kernel memory.
 *
 * This array is placed in the kernel's .bss section.
 */
static unsigned char kernel_heap[HEAP_SIZE]
    __attribute__((aligned(MEMORY_ALIGNMENT)));

/*
 * Number of bytes already used from the heap.
 */
static unsigned int heap_offset = 0;


/*
 * Rounds a size upward to the next 16-byte boundary.
 *
 * Examples:
 *   1 byte  becomes 16 bytes
 *   16 bytes stays 16 bytes
 *   17 bytes becomes 32 bytes
 */
static unsigned int align_size(unsigned int size)
{
    return (
        size + MEMORY_ALIGNMENT - 1
    ) & ~(MEMORY_ALIGNMENT - 1);
}


/*
 * Resets the heap so that no memory is marked as used.
 */
void memory_initialize(void)
{
    heap_offset = 0;
}


/*
 * Reserves a block of memory and returns its address.
 */
void *kmalloc(unsigned int size)
{
    if (size == 0)
    {
        return (void *)0;
    }

    unsigned int aligned_size = align_size(size);

    /*
     * Check whether the requested block fits.
     *
     * Written this way to avoid integer overflow.
     */
    if (aligned_size > HEAP_SIZE - heap_offset)
    {
        return (void *)0;
    }

    /*
     * Point to the beginning of the unused section.
     */
    void *allocated_memory =
        (void *)&kernel_heap[heap_offset];

    /*
     * Move the heap position forward.
     */
    heap_offset += aligned_size;

    return allocated_memory;
}


/*
 * Returns the number of allocated bytes.
 */
unsigned int memory_get_used(void)
{
    return heap_offset;
}


/*
 * Returns the number of available bytes.
 */
unsigned int memory_get_free(void)
{
    return HEAP_SIZE - heap_offset;
}
#include "multiboot.h"
#include "graphics.h"
#include "font.h"
#include "title_screen.h"
#include "game_select.h"
#include "pong.h"
#include "keyboard.h"
#include "interrupts.h"
#include "timer.h"
#include "memory.h"
#include "io.h"
#include "sound.h"


#define TIMER_FREQUENCY 100


/*
 * Sends debugging text to QEMU's debug console.
 *
 * The text appears in the WSL terminal because QEMU
 * is started with:
 *
 *     -debugcon stdio
 */
static void debug_write(const char *text)
{
    uint32_t index = 0;

    while (text[index] != '\0')
    {
        outb(
            0xE9,
            (unsigned char)text[index]
        );

        index++;
    }
}


/*
 * Permanently stops the processor after a fatal error.
 */
static void halt_forever(void)
{
    /*
     * Disable hardware interrupts.
     */
    __asm__ volatile ("cli");

    while (1)
    {
        /*
         * Stop the processor until an interrupt occurs.
         *
         * Since interrupts are disabled here, the
         * processor remains halted.
         */
        __asm__ volatile ("hlt");
    }
}


/*
 * ArcadeOS begins execution here after GRUB loads
 * the kernel.
 *
 * multiboot_magic:
 *     proves GRUB used the Multiboot protocol
 *
 * multiboot_info_address:
 *     address of GRUB's Multiboot information,
 *     including framebuffer details
 */
void kernel_main(
    uint32_t multiboot_magic,
    uint32_t multiboot_info_address
)
{
    debug_write(
        "ArcadeOS: kernel_main entered\n"
    );


    /*
     * Prepare ArcadeOS's simple kernel memory allocator.
     */
    memory_initialize();


    /*
     * Verify that a Multiboot-compatible bootloader
     * started ArcadeOS.
     */
    if (
        multiboot_magic
        != MULTIBOOT_BOOTLOADER_MAGIC
    )
    {
        debug_write(
            "ArcadeOS error: invalid Multiboot magic\n"
        );

        halt_forever();
    }


    debug_write(
        "ArcadeOS: Multiboot magic valid\n"
    );


    /*
     * GRUB passed the Multiboot information as a
     * numeric memory address.
     *
     * Convert that address into a pointer so the
     * kernel can access the structure.
     */
    multiboot_info_t *multiboot_info =
        (multiboot_info_t *)
        multiboot_info_address;


    /*
     * Connect ArcadeOS's graphics driver to the
     * framebuffer supplied by GRUB.
     */
    if (!graphics_initialize(multiboot_info))
    {
        debug_write(
            "ArcadeOS error: graphics initialization failed\n"
        );

        halt_forever();
    }


    debug_write(
        "ArcadeOS: framebuffer initialized\n"
    );


    /*
     * Create the Interrupt Descriptor Table and
     * configure the Programmable Interrupt Controller.
     */
    interrupts_initialize();


    /*
     * Configure the Programmable Interval Timer to
     * interrupt the processor 100 times per second.
     *
     * These timer ticks control menu animations and
     * Pong's game speed.
     */
    timer_initialize(
        TIMER_FREQUENCY
    );

    sound_initialize();

    


    /*
     * Allow hardware interrupts to reach the processor.
     */
    interrupts_enable();

    


    debug_write(
        "ArcadeOS: systems initialized\n"
    );


    /*
     * Main ArcadeOS screen loop.
     *
     * The kernel continues switching between:
     *
     *     title screen
     *     game selector
     *     selected games
     */
    while (1) {
    /*
     * Temporary speaker test:
     * play a continuous tone while the title screen is open.
     */
    

    title_screen_run();

    /*
     * Stop the tone after Enter leaves the title screen.
     */
    

    debug_write("ArcadeOS: entering game selector\n");
        /*
         * Display the ArcadeOS title screen.
         *
         * This function returns when the user presses
         * Enter to insert a credit.
         */
        title_screen_run();


        debug_write(
            "ArcadeOS: entering game selector\n"
        );


        /*
         * Remain inside the arcade until Escape is
         * pressed from the main game selector.
         */
        while (1)
        {
            game_selection_t selection =
                game_select_run();


            /*
             * Escape from the game selector returns
             * to the ArcadeOS title screen.
             */
            if (
                selection
                == GAME_SELECTION_BACK
            )
            {
                debug_write(
                    "ArcadeOS: returning to title screen\n"
                );

                break;
            }


            /*
             * Enter on the Pong cabinet launches Pong.
             *
             * pong_run() contains:
             *
             *     one-player mode selection
             *     two-player mode selection
             *     CPU paddle logic
             *     Pong matches
             *     rematches
             *     returning to the selector
             */
            if (
                selection
                == GAME_SELECTION_PONG
            )
            {
                debug_write(
                    "ArcadeOS: launching Pong\n"
                );

                pong_run();

                debug_write(
                    "ArcadeOS: returned from Pong\n"
                );
            }
        }
    }
}
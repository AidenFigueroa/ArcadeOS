#include "sound.h"
#include "io.h"
#include "volume.h"

/*
 * The Programmable Interval Timer receives a clock signal
 * approximately 1,193,182 times per second.
 */
#define PIT_BASE_FREQUENCY 1193182U

/*
 * PIT channel 2 controls the PC speaker frequency.
 */
#define PIT_CHANNEL_2_PORT 0x42

/*
 * The PIT command port selects which timer channel
 * we want to configure.
 */
#define PIT_COMMAND_PORT 0x43

/*
 * This port enables or disables the PC speaker.
 */
#define PC_SPEAKER_PORT 0x61

void sound_initialize(void)
{
    /*
     * Make sure the speaker is silent when ArcadeOS starts.
     */
    sound_stop();
}

void sound_play_tone(uint32_t frequency)
{
    uint32_t divisor;
    uint8_t speaker_state;

    /*
     * Level zero is a real global mute.
     */
    if (volume_is_muted())
    {
        sound_stop();
        return;
    }

    /*
     * A frequency of zero cannot produce a valid tone.
     * Treat it as a request for silence.
     */
    if (frequency == 0) {
        sound_stop();
        return;
    }

    /*
     * Convert the requested frequency into the value
     * expected by the PIT.
     *
     * Example:
     *
     *     divisor = 1193182 / 440
     */
    divisor = PIT_BASE_FREQUENCY / frequency;

    /*
     * The PIT divisor is only 16 bits.
     */
    if (divisor < 1) {
        divisor = 1;
    }

    if (divisor > 65535) {
        divisor = 65535;
    }

    /*
     * 0xB6 means:
     *
     * - Use PIT channel 2
     * - Send the divisor as a low byte followed by a high byte
     * - Use square-wave mode
     * - Use binary counting
     */
    outb(PIT_COMMAND_PORT, 0xB6);

    /*
     * Send the lower 8 bits of the divisor.
     */
    outb(
        PIT_CHANNEL_2_PORT,
        (uint8_t)(divisor & 0xFF)
    );

    /*
     * Send the upper 8 bits of the divisor.
     */
    outb(
        PIT_CHANNEL_2_PORT,
        (uint8_t)((divisor >> 8) & 0xFF)
    );

    /*
     * Read the current PC speaker control value.
     */
    speaker_state = inb(PC_SPEAKER_PORT);

    /*
     * Bits 0 and 1 must both be enabled for sound.
     */
    if ((speaker_state & 0x03) != 0x03) {
        outb(
            PC_SPEAKER_PORT,
            speaker_state | 0x03
        );
    }
}

void sound_stop(void)
{
    uint8_t speaker_state;

    /*
     * Read the current speaker control value.
     */
    speaker_state = inb(PC_SPEAKER_PORT);

    /*
     * Clear bits 0 and 1 while leaving every other
     * hardware-control bit unchanged.
     */
    outb(
        PC_SPEAKER_PORT,
        speaker_state & 0xFC
    );
}

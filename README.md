# ArcadeOS

**A bootable 32-bit x86 arcade operating system written from scratch in C and NASM assembly.**

ArcadeOS boots through GRUB, initializes its own low-level hardware services, renders a custom graphical interface, accepts raw keyboard input, produces PC-speaker audio, and runs a complete version of Pong without relying on a host operating system, game engine, graphics library, or audio framework.

The system was developed on an ASUS ROG Strix, tested in QEMU, packaged as a bootable ISO, and deployed by USB to a legacy Dell Inspiron 5150.

## Project Highlights

* Built a freestanding 32-bit x86 kernel using C and NASM
* Configured protected mode, the Global Descriptor Table, and segment registers
* Implemented interrupt handling and Programmable Interval Timer support
* Created keyboard, framebuffer graphics, bitmap-font, and PC-speaker subsystems
* Built a title screen, game-selection interface, and playable Pong game
* Added one-player and two-player modes with predictive CPU behavior
* Packaged the project as a bootable ISO for QEMU and physical x86 hardware
* Diagnosed and fixed a rendering bottleneck that appeared only on legacy hardware

## Why I Built It

I built ArcadeOS to move beyond studying operating-system concepts only in theory.

A normal application can depend on an operating system, window manager, graphics API, audio library, and input framework. ArcadeOS cannot. The project has to establish the execution environment and provide the services its game needs directly.

That required me to work through the complete path from machine startup to an interactive program:

1. Boot the kernel through GRUB
2. Enter a 32-bit protected-mode environment
3. Configure processor segments and linked memory sections
4. Initialize interrupts and timer hardware
5. Read keyboard state through port-mapped input
6. Write pixels directly to a linear framebuffer
7. Generate sound through the PC speaker
8. Coordinate input, rendering, audio, timing, and game logic in a real-time loop

The result is intentionally small in scope, but complete enough to boot, interact with hardware, and run a testable application.

## System Architecture

```text
GRUB
  |
  v
boot/boot.asm
  |
  v
32-bit kernel entry
  |
  +--> GDT and segment initialization
  +--> Interrupt Descriptor Table and IRQ handling
  +--> Programmable Interval Timer
  +--> Keyboard input
  +--> Framebuffer graphics
  +--> Bitmap-font rendering
  +--> PC-speaker audio
  |
  v
Title Screen
  |
  v
Game Selector
  |
  v
Pong
```

## Core Systems

| Subsystem     | Implementation                                         |
| ------------- | ------------------------------------------------------ |
| Boot          | GRUB Multiboot entry with custom assembly startup code |
| CPU setup     | 32-bit x86 protected mode and Global Descriptor Table  |
| Memory layout | Custom linker script and freestanding kernel sections  |
| Interrupts    | Interrupt setup with assembly stubs and C handlers     |
| Timing        | PIT channel 0 provides the system tick                 |
| Input         | Raw keyboard scancode handling                         |
| Graphics      | 800×600, 32-bit linear framebuffer                     |
| Text          | Custom bitmap-font renderer                            |
| Audio         | PIT channel 2 and PC-speaker tone generation           |
| Application   | Menu system and real-time Pong game loop               |

## Physical Hardware Deployment

ArcadeOS was first tested in QEMU and then deployed to a Dell Inspiron 5150 using a bootable USB drive.

Running on physical hardware exposed a performance problem that was not visible in the emulator.

### Rendering Bottleneck

The original Pong renderer used double buffering and copied the complete 800×600×32-bit back buffer into video memory for every displayed frame.

Each full-screen copy moved approximately **1.92 MB of pixel data per frame**. QEMU handled this smoothly, but the Dell Inspiron 5150 could not transfer full frames to video memory quickly enough, causing severe gameplay lag.

I traced the problem to framebuffer bandwidth rather than the game physics or timer logic.

### Rendering Redesign

I replaced full-frame copying during gameplay with incremental direct rendering:

* Erase the previous ball and paddle positions
* Restore permanent court elements that were overwritten
* Draw the objects at their new positions
* Leave unchanged regions of the framebuffer untouched

This reduced unnecessary video-memory writes and made Pong playable on the physical laptop.

The approach can produce minor frame tearing on older hardware. I accepted and documented that tradeoff because the performance improvement was more important for the project's supported target system.

This was the project's most valuable engineering lesson: code that works in an emulator is not necessarily correct or efficient for real hardware.

## Pong

Pong runs directly inside the kernel and uses the operating-system services implemented by the project.

### Features

* One-player mode against a CPU-controlled paddle
* Two-player local multiplayer
* Predictive CPU ball tracking with a reaction delay
* Paddle and wall collision handling
* Ball-angle changes based on impact position
* Increasing ball speed during longer rallies
* First-to-seven scoring
* Score, victory, rematch, and mode-selection screens
* Nonblocking sound effects driven by timer ticks
* Incremental rendering optimized for legacy hardware

### Controls

| Mode       | Control                 | Action             |
| ---------- | ----------------------- | ------------------ |
| Menus      | Arrow keys or `W` / `S` | Move selection     |
| Menus      | `Enter`                 | Confirm            |
| Menus      | `Esc`                   | Go back            |
| One player | Up / Down arrows        | Move player paddle |
| Two player | Up / Down arrows        | Move Player One    |
| Two player | `W` / `S`               | Move Player Two    |

## Audio

ArcadeOS programs the PC speaker through PIT channel 2 while reserving PIT channel 0 for system timer interrupts.

The audio system includes:

* Looping title-screen music
* Game-selection music
* Menu movement and confirmation sounds
* Pong wall, paddle, score, and victory effects
* Timer-driven, nonblocking playback

Keeping the timer and speaker channels separate prevents audio playback from changing the operating system's main timing frequency.

## Project Structure

```text
ArcadeOS/
├── boot/
│   ├── boot.asm              # Kernel entry and protected-mode setup
│   └── interrupts.asm        # Low-level interrupt stubs
├── iso/boot/grub/
│   └── grub.cfg              # GRUB configuration
├── kernel.c                  # Kernel initialization and main control flow
├── interrupts.c/.h           # Interrupt setup and handlers
├── timer.c/.h                # PIT timing services
├── keyboard.c/.h             # Keyboard input
├── graphics.c/.h             # Framebuffer drawing
├── font.c/.h                 # Bitmap-font rendering
├── title_screen.c/.h         # Title-screen interface
├── game_select.c/.h          # Game-selection interface
├── pong.c/.h                 # Pong logic and rendering
├── sound.c/.h                # PC-speaker sound effects
├── music.c/.h                # Music sequencing
├── memory.c/.h               # Freestanding memory utilities
├── terminal.c/.h             # Terminal support
├── multiboot.h               # Multiboot structures
├── linker.ld                 # Kernel memory layout
└── Makefile                  # Build and QEMU commands
```

## Build and Run

The following commands assume Ubuntu or Windows Subsystem for Linux.

### Install Dependencies

```bash
sudo apt update

sudo apt install \
    build-essential \
    gcc-multilib \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    make
```

### Build the Bootable ISO

```bash
make clean
make ArcadeOS.iso
```

### Run in QEMU

```bash
make run
```

The generated boot image is:

```text
ArcadeOS.iso
```

## Boot on Physical Hardware

> **Warning:** Writing an ISO to a USB drive erases the selected drive. Verify the target device before continuing.

ArcadeOS was deployed with Rufus using:

* **Partition scheme:** MBR
* **Target system:** BIOS or UEFI-CSM
* **File system:** FAT32

The target machine must support the hardware interfaces currently used by ArcadeOS:

* 32-bit x86
* Legacy BIOS-compatible booting
* GRUB Multiboot
* Compatible linear framebuffer
* Standard PC keyboard
* Programmable Interval Timer
* PC speaker

ArcadeOS does not currently support other architectures such as ARM or RISC-V. Those targets would require separate boot code, memory layouts, processor setup, and device drivers.

## Technical Challenges

### Booting Without a Host Operating System

ArcadeOS cannot use normal application-level services. It provides its own startup code, linker organization, kernel entry point, processor setup, hardware initialization, and runtime control flow.

### Interrupt-Driven Timing

The PIT tick counter coordinates:

* Game updates
* Music playback
* Sound effects
* CPU reaction timing
* Animation timing

### Emulator Versus Hardware Behavior

QEMU enabled fast iteration, but physical deployment revealed a framebuffer bottleneck that required an architectural rendering change rather than a superficial adjustment to game-speed values.

### Shared Timer Hardware

PIT channel 0 drives operating-system timing, while PIT channel 2 generates PC-speaker tones. Separating these responsibilities keeps audio playback from disrupting the game loop.

## Skills Demonstrated

* Bare-metal and freestanding C
* x86 NASM assembly
* Bootloaders and GRUB Multiboot
* Linker scripts and binary layout
* Protected-mode and segment initialization
* Hardware interrupts
* Port-mapped input and output
* Timer-driven programming
* Keyboard input handling
* Framebuffer graphics
* PC-speaker audio
* Real-time game-loop design
* Emulator-based testing
* Physical-hardware deployment
* Performance debugging under hardware constraints
* Engineering tradeoff analysis and documentation

## Current Scope

ArcadeOS is an educational operating system focused on low-level startup, hardware interaction, graphics, input, sound, timing, and an interactive game.

It is not intended to be a general-purpose desktop operating system. Its purpose is to demonstrate a complete, bootable, and testable systems-programming project.

## Future Work

Potential improvements include:

* Dirty-rectangle rendering
* Hardware capability detection
* Additional graphics modes
* Improved memory management
* Filesystem support
* Additional device drivers
* A reusable game API
* Additional arcade games
* Persistent high scores
* Testing across more x86 hardware

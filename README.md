ArcadeOS

A bootable 32-bit x86 arcade operating system written in C and NASM assembly.

Developed on an ASUS ROG Strix, tested in QEMU, packaged as a bootable ISO, and deployed by USB to a legacy Dell Inspiron 5150.

Portfolio Demo

Overview

ArcadeOS is a freestanding hobby operating system built to explore how software interacts directly with a computer when there is no host operating system underneath it.

The project boots through GRUB, enters a 32-bit x86 environment, initializes the hardware services needed by the kernel, displays a custom framebuffer interface, reads keyboard input, produces PC-speaker audio, and runs a playable version of Pong.

ArcadeOS currently includes:

A custom boot process and linker layout

32-bit x86 protected-mode setup

Global Descriptor Table setup

Interrupt handling

Programmable Interval Timer support

Keyboard input

800×600 32-bit framebuffer graphics

Bitmap-font text rendering

PC-speaker music and sound effects

A title screen and game-selection interface

One-player and two-player Pong

Predictive CPU paddle behavior

Scoring, collisions, increasing ball speed, and win screens

QEMU support

Booting on physical x86 hardware

Why I Built It

I created ArcadeOS to move beyond learning operating-system concepts only in theory.

Instead of using an existing engine, windowing system, graphics library, or audio framework, I had to build the low-level systems that the game depends on:

Booting the kernel

Organizing memory and linked sections

Configuring processor segments

Handling hardware interrupts

Tracking time with the PIT

Reading raw keyboard state

Writing pixels directly to a framebuffer

Programming the PC speaker

Managing a real-time game loop

The result is deliberately small, but it demonstrates the complete path from booting a machine to running an interactive program.

Physical Hardware Deployment

ArcadeOS was developed on an ASUS ROG Strix and initially tested with QEMU.

After the operating system worked in the emulator, I generated a bootable ISO, wrote it to a USB drive, and booted it on a separate Dell Inspiron 5150.

This step mattered because an emulator cannot fully reproduce the timing, bandwidth, and behavior of every physical machine.

Hardware-specific problem discovered

The original Pong renderer used double buffering and copied the complete 800×600×32-bit back buffer into video memory for each displayed frame.

That approach worked smoothly in QEMU, but it caused severe lag on the Dell Inspiron 5150 because the older machine could not transfer full frames to its framebuffer quickly enough.

I diagnosed the rendering bottleneck and changed Pong to use incremental direct rendering:

Erase the previous ball and paddle positions.

Restore any permanent court elements that were overwritten.

Draw the objects at their new positions.

Avoid copying the entire framebuffer every frame.

This significantly reduced the amount of video memory written during gameplay and allowed Pong to run properly on the physical laptop.

Direct rendering can produce minor frame tearing on the older machine. I accepted that tradeoff because it improves performance and also fits the project's retro arcade presentation.

What this demonstrates

Moving ArcadeOS from the development laptop to a separate physical computer demonstrates an important part of systems programming: software must work within real hardware constraints rather than only inside the environment where it was created.

More precisely, this project demonstrates portability across different machines within ArcadeOS's supported platform:

32-bit x86

Legacy BIOS-compatible booting

GRUB Multiboot support

A compatible linear framebuffer

Standard PC keyboard, PIT, and PC-speaker hardware interfaces

It does not currently claim cross-architecture portability to systems such as ARM or RISC-V. Supporting those architectures would require separate boot code, hardware drivers, memory layouts, and processor-specific implementations.

System Architecture

GRUB
  |
  v
boot/boot.asm
  |
  v
32-bit kernel entry
  |
  +--> GDT and segment setup
  +--> Interrupt initialization
  +--> PIT timer
  +--> Keyboard input
  +--> Framebuffer graphics
  +--> Font rendering
  +--> PC-speaker sound
  |
  v
Title Screen
  |
  v
Game Selector
  |
  v
Pong

Pong

ArcadeOS includes a complete Pong game running directly inside the kernel.

Game features

One-player mode against a CPU paddle

Two-player local mode

First player to seven points wins

Paddle and wall collision handling

Ball-angle changes based on paddle impact position

Ball-speed progression during rallies

Score display

Rematch and mode-selection screens

Nonblocking PC-speaker sound effects

CPU reaction delay and predictive ball tracking

Direct incremental rendering for legacy hardware

Controls

General menus

Key

Action

Arrow keys or W / S

Change the highlighted option

Enter

Confirm

Esc

Return to the previous screen

One-player Pong

Key

Action

Up Arrow

Move Player One paddle up

Down Arrow

Move Player One paddle down

Esc

Return to mode selection

Two-player Pong

Player

Up

Down

Player One

Up Arrow

Down Arrow

Player Two

W

S

Audio

ArcadeOS programs the PC speaker through PIT channel 2.

The audio system supports:

A looping title-screen theme

A calmer game-selection melody

Menu movement sounds

Confirmation sounds

Back-navigation sounds

Pong wall, paddle, score, and victory effects

Audio playback is advanced using timer ticks so effects can play without blocking the game loop.

Project Structure

ArcadeOS/
├── boot/
│   ├── boot.asm
│   └── interrupts.asm
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg
├── kernel.c
├── terminal.c
├── terminal.h
├── keyboard.c
├── keyboard.h
├── io.h
├── interrupts.c
├── interrupts.h
├── timer.c
├── timer.h
├── memory.c
├── memory.h
├── graphics.c
├── graphics.h
├── font.c
├── font.h
├── title_screen.c
├── title_screen.h
├── game_select.c
├── game_select.h
├── pong.c
├── pong.h
├── sound.c
├── sound.h
├── music.c
├── music.h
├── multiboot.h
├── linker.ld
└── Makefile

Development Environment

ArcadeOS was developed using:

Windows on an ASUS ROG Strix

Windows Subsystem for Linux

GCC

GNU linker

NASM

GNU Make

GRUB utilities

xorriso

QEMU

Rufus for writing the ISO to USB

Building ArcadeOS

The commands below assume an Ubuntu or WSL-based development environment.

Required tools

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

Build the ISO

From the project directory:

make clean
make ArcadeOS.iso

The generated boot image will be:

ArcadeOS.iso

Run in QEMU

make run

Installing on a USB Drive

Warning: writing the ISO will erase the selected USB drive.

ArcadeOS was written to USB using Rufus with:

Partition scheme: MBR
Target system: BIOS or UEFI-CSM
File system: FAT32

Basic process:

Build ArcadeOS.iso.

Open Rufus.

Select the correct USB drive.

Select ArcadeOS.iso.

Start the write process.

Wait until Rufus reports READY.

Safely eject the USB.

Insert it into the target computer.

Open the computer's boot menu.

Select the USB device.

Technical Challenges

Booting without an existing operating system

ArcadeOS cannot rely on normal application services. The project has to provide its own startup code, linker organization, kernel entry point, hardware setup, and runtime behavior.

Interrupt-driven timing

The system timer provides a stable tick counter used by:

Game updates

Music playback

Sound effects

CPU reaction timing

Animation timing

Emulator versus physical hardware

QEMU was useful for rapid development, but the Dell Inspiron 5150 exposed a framebuffer-performance problem that was not obvious in the emulator.

Solving that issue required changing the rendering strategy rather than merely increasing game speed values. The bottleneck was the amount of graphics memory transferred, not the Pong physics.

Shared PIT hardware

ArcadeOS uses:

PIT channel 0 for timer interrupts

PIT channel 2 for PC-speaker tones

Keeping these channels separate prevents audio code from changing the operating system's main timer frequency.

What I Learned

This project helped me develop practical experience with:

Bare-metal C

x86 assembly

Bootloaders and Multiboot

Linker scripts

Protected-mode setup

Hardware interrupts

Port-mapped input and output

Timer-based programming

Keyboard drivers

Framebuffer graphics

PC-speaker audio

Real-time game loops

Debugging emulator-specific assumptions

Optimizing software for older physical hardware

Designing software around supported hardware interfaces

The most important lesson was that getting code to run in an emulator is not the end of systems work. A system must also handle the behavior and limitations of the hardware on which it is expected to run.

Current Scope

ArcadeOS is an educational hobby operating system focused on booting, hardware interaction, graphics, sound, input, and an interactive game.

It is not intended to be a general-purpose desktop operating system. Its current purpose is to demonstrate low-level systems programming through a complete, bootable, and testable project.

Future Improvements

Possible future additions include:

Additional arcade games

A reusable game API

More efficient dirty-rectangle rendering

Hardware capability detection

Multiple graphics modes

Improved memory management

Filesystem support

Additional device drivers

A settings screen

Persistent high scores

More extensive testing on different x86 computers
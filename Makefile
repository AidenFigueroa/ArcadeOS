ASM = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

OBJECTS = \
	boot.o \
	interrupt_stubs.o \
	kernel.o \
	terminal.o \
	keyboard.o \
	interrupts.o \
	timer.o \
	memory.o \
	graphics.o \
	font.o \
	sound.o \
	music.o \
	volume.o \
	title_screen.o \
	game_select.o \
	pong.o

all: ArcadeOS.iso

boot.o: boot/boot.asm
	$(ASM) -f elf32 boot/boot.asm -o boot.o

interrupt_stubs.o: boot/interrupts.asm
	$(ASM) -f elf32 boot/interrupts.asm -o interrupt_stubs.o

kernel.o: kernel.c multiboot.h graphics.h font.h title_screen.h game_select.h pong.h keyboard.h interrupts.h timer.h memory.h io.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

pong.o: pong.c pong.h graphics.h font.h keyboard.h timer.h music.h volume.h
	$(CC) $(CFLAGS) -c pong.c -o pong.o

game_select.o: game_select.c game_select.h graphics.h font.h keyboard.h timer.h music.h volume.h
	$(CC) $(CFLAGS) -c game_select.c -o game_select.o

title_screen.o: title_screen.c title_screen.h graphics.h font.h keyboard.h timer.h multiboot.h music.h volume.h
	$(CC) $(CFLAGS) -c title_screen.c -o title_screen.o

font.o: font.c font.h graphics.h multiboot.h
	$(CC) $(CFLAGS) -c font.c -o font.o

terminal.o: terminal.c terminal.h
	$(CC) $(CFLAGS) -c terminal.c -o terminal.o

keyboard.o: keyboard.c keyboard.h io.h volume.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

interrupts.o: interrupts.c interrupts.h io.h
	$(CC) $(CFLAGS) -c interrupts.c -o interrupts.o

timer.o: timer.c timer.h interrupts.h io.h
	$(CC) $(CFLAGS) -c timer.c -o timer.o

memory.o: memory.c memory.h
	$(CC) $(CFLAGS) -c memory.c -o memory.o

graphics.o: graphics.c graphics.h multiboot.h
	$(CC) $(CFLAGS) -c graphics.c -o graphics.o

sound.o: sound.c sound.h io.h volume.h
	$(CC) $(CFLAGS) -c sound.c -o sound.o

music.o: music.c music.h sound.h timer.h
	$(CC) $(CFLAGS) -c music.c -o music.o

volume.o: volume.c volume.h graphics.h font.h sound.h keyboard.h
	$(CC) $(CFLAGS) -c volume.c -o volume.o

kernel.bin: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJECTS)
	grub-file --is-x86-multiboot kernel.bin

iso/boot/kernel.bin: kernel.bin
	cp kernel.bin iso/boot/kernel.bin

ArcadeOS.iso: iso/boot/kernel.bin iso/boot/grub/grub.cfg
	grub-mkrescue -o ArcadeOS.iso iso

run: ArcadeOS.iso
	qemu-system-i386 -cdrom ArcadeOS.iso -vga std -display gtk,zoom-to-fit=off -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -debugcon stdio -no-reboot -no-shutdown

clean:
	rm -f $(OBJECTS) kernel.bin iso/boot/kernel.bin ArcadeOS.iso

.PHONY: all run clean

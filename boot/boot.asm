BITS 32

; Multiboot header constants.
MAGIC     equ 0x1BADB002
FLAGS     equ 0x00000004
CHECKSUM  equ -(MAGIC + FLAGS)


section .multiboot
align 4

    dd MAGIC
    dd FLAGS
    dd CHECKSUM

    ; Address-field placeholders.
    ;
    ; ArcadeOS is an ELF kernel, so GRUB gets the
    ; loading information from the ELF file instead.
    ; These values remain zero, but they must occupy
    ; their positions so the graphics fields begin
    ; at byte offset 32.
    dd 0                ; header_addr
    dd 0                ; load_addr
    dd 0                ; load_end_addr
    dd 0                ; bss_end_addr
    dd 0                ; entry_addr

    ; Preferred graphics mode.
    dd 0                ; linear graphics mode
    dd 800              ; width
    dd 600              ; height
    dd 32               ; bits per pixel


section .bss
align 16

stack_bottom:
    resb 16384
stack_top:


section .rodata
align 8

gdt_start:

gdt_null:
    dq 0x0000000000000000

gdt_code:
    dq 0x00CF9A000000FFFF

gdt_data:
    dq 0x00CF92000000FFFF

gdt_end:


gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


section .text

global _start
extern kernel_main


_start:
    ; GRUB places the Multiboot boot magic in EAX
    ; and the Multiboot information address in EBX.
    mov esi, eax
    mov edi, ebx

    cli

    ; Load ArcadeOS's GDT.
    lgdt [gdt_descriptor]

    ; Reload the code segment.
    jmp CODE_SEG:.reload_segments


.reload_segments:
    ; Reload the data segments.
    mov ax, DATA_SEG

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the kernel stack.
    mov esp, stack_top
    and esp, 0xFFFFFFF0

    ; Keep the stack aligned while passing two arguments.
    sub esp, 8

    ; C arguments are pushed right to left.
    push edi             ; multiboot_info_address
    push esi             ; multiboot_magic

    call kernel_main

    add esp, 16


.hang:
    cli
    hlt
    jmp .hang
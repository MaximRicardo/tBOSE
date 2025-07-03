;First stage of the boot loader

[org 0x7c00]

;The second bootloader stage will be placed right after the first
%define SECOND_STAGE_LOCATION 0x7d00
%define SECOND_STAGE_SECTORS_TO_READ 4

segment .text

;Start of the entire OS
;BIOS hands over control to the boot loader from here
start:
    jmp BootLoaderStart
    TIMES 3-($-$$) DB 0x90   ; Support 2 or 3 byte encoded JMPs before BPB.

    ; Dos 4.0 EBPB 1.44MB floppy
BPB:
    .OEMname:           db    "mkfs.fat"  ; mkfs.fat is what OEMname mkdosfs uses
    .bytesPerSector:    dw    512
    .sectPerCluster:    db    1
    .reservedSectors:   dw    1
    .numFAT:            db    2
    .numRootDirEntries: dw    224
    .numSectors:        dw    2880
    .mediaType:         db    0xf0
    .numFATsectors:     dw    9
    .sectorsPerTrack:   dw    18
    .numHeads:          dw    2
    .numHiddenSectors:  dd    0
    .numSectorsHuge:    dd    0
    .driveNum:          db    0
    .reserved:          db    0
    .signature:         db    0x29
    .volumeID:          dd    0x2d7e5a1a
    .volumeLabel:       db    "NO NAME    "
    .fileSysType:       db    "FAT12   "

BootLoaderStart:
    ;Setup the segment registers
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov [BOOT_DISK], dl

    ;Setup the stack
    mov bp, 0x0500
    mov sp, bp

    ;Print the msg
    mov si, msg
    call PrintStr

LoadingSecondStage:
    ;Load in the second bootloader stage
    mov bx, SECOND_STAGE_LOCATION
    mov ah, 2
    mov al, SECOND_STAGE_SECTORS_TO_READ
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [BOOT_DISK]
    int 0x13

    jc SecondStageLoadError ;The carry flag is set if there is an error
    cmp al, SECOND_STAGE_SECTORS_TO_READ
    jne SecondStageLoadError  ;If the incorrect number of sectors were read, then thats a problem

    mov si, second_stage_load_success_msg
    call PrintStr

    ;Jump into the second bootloader stage
    ;Also, pass the boot disk in dl
    mov dl, [BOOT_DISK]
    jmp SECOND_STAGE_LOCATION

SecondStageLoadError:

    inc byte[n_second_stage_load_attempts]

    mov al, n_second_stage_load_attempts
    ;Attempt to load the second stage four times before giving up
    ;This is because floppy disks are typically pretty bad at loading stuff, and
    ;multiple attemps may be needed.
    cmp al, 4
    jle LoadingSecondStage

    mov si, second_stage_load_error_msg
    call PrintStr

    jmp HaltLoop

HaltLoop:
    hlt
    jmp HaltLoop

;Prints a null-terminated string
;Pointer to the string is must be passed in the si register
;Preserves segment registers
PrintStr:
    
    .PrintStr_Loop:
        mov al, [si]  ;Move the current character into al to be printed by the BIOS
        
        ;Stop at a null terminator
        cmp al, 0
        je .PrintStr_LoopEnd

        ;Setup the BIOS interrupt argument
        mov ah, 0x0e
        ;Set the page to 0
        xor bx, bx
        ;Print the character
        int 0x10

        ;Move to the next character
        inc si

        jmp .PrintStr_Loop

    .PrintStr_LoopEnd:

    ret

BOOT_DISK: db 0

;The number of times loading the second stage of the bootloader has been attempted and failed
n_second_stage_load_attempts: db 0

msg: db "First bootloader stage running!", 0x0a, 0x0d, "Loading second stage...", 0x0a, 0x0d, 0x0

second_stage_load_error_msg:
    db "ERROR: Failed to load second bootloader stage!", 0x0a, 0x0d, 0x0

second_stage_load_success_msg:
    db "Loaded the second bootloader stage!", 0x0a, 0x0d, 0x0

times 510-($-$$) db 0
dw 0xaa55

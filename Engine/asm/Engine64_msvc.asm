; ==========================================================================
; Engine64_msvc.asm - Engine pixel/color routines, Windows x64 (MS ABI)
; Assembled by ml64; GCC/Clang version: Engine64_gnu.S
; See Engine/AsmRoutines.h for the argument contract.
; ==========================================================================

.code

; --------------------------------------------------------------------------
; void AsmDrawPixel16(const AsmPixelArgs* args)      ; rcx = args
; --------------------------------------------------------------------------
public AsmDrawPixel16
AsmDrawPixel16 proc
    mov     eax, dword ptr [rcx+16]     ; y
    mul     dword ptr [rcx+8]           ; y * pitch
    mov     r10d, dword ptr [rcx+12]    ; x
    add     eax, r10d
    add     eax, r10d                   ; offset = y*pitch + 2*x
    mov     r11, qword ptr [rcx]        ; frameBuffer
    add     r11, rax
    mov     ax, word ptr [rcx+20]       ; color
    mov     word ptr [r11], ax
    ret
AsmDrawPixel16 endp

; --------------------------------------------------------------------------
; void AsmDrawPixelAlpha16(const AsmPixelArgs* args) ; rcx = args
; blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f dual-word expansion
; --------------------------------------------------------------------------
public AsmDrawPixelAlpha16
AsmDrawPixelAlpha16 proc
    mov     eax, dword ptr [rcx+16]     ; y
    mul     dword ptr [rcx+8]           ; y * pitch
    mov     r10d, dword ptr [rcx+12]    ; x
    add     eax, r10d
    add     eax, r10d
    mov     r11, qword ptr [rcx]
    add     r11, rax                    ; r11 = target pixel address

    mov     eax, dword ptr [rcx+20]     ; color
    mov     r10d, eax
    shl     eax, 16
    mov     ax, r10w
    and     eax, 07e0f81fh              ; src expanded

    movzx   r10d, word ptr [r11]        ; dst
    mov     r8d, r10d
    shl     r10d, 16
    mov     r10w, r8w
    and     r10d, 07e0f81fh             ; dst expanded

    mov     r8d, dword ptr [rcx+24]     ; alpha
    mul     r8d                         ; src * alpha
    neg     r8d
    add     r8d, 32                     ; 32 - alpha
    xchg    eax, r10d
    mul     r8d                         ; dst * (32 - alpha)
    add     eax, r10d
    shr     eax, 5
    and     eax, 07e0f81fh
    mov     r10w, ax
    shr     eax, 16
    or      ax, r10w
    mov     word ptr [r11], ax
    ret
AsmDrawPixelAlpha16 endp

; --------------------------------------------------------------------------
; unsigned short AsmRGB555(int r, int g, int b)      ; ecx/edx/r8d
; --------------------------------------------------------------------------
public AsmRGB555
AsmRGB555 proc
    mov     eax, ecx
    and     eax, 0ffh
    shr     eax, 3
    shl     eax, 10
    and     edx, 0ffh
    shr     edx, 3
    shl     edx, 5
    or      eax, edx
    and     r8d, 0ffh
    shr     r8d, 3
    or      eax, r8d
    ret                                 ; result in ax
AsmRGB555 endp

; --------------------------------------------------------------------------
; unsigned short AsmRGB565(int r, int g, int b)      ; ecx/edx/r8d
; --------------------------------------------------------------------------
public AsmRGB565
AsmRGB565 proc
    mov     eax, ecx
    and     eax, 0ffh
    shr     eax, 3
    shl     eax, 11
    and     edx, 0ffh
    shr     edx, 2
    shl     edx, 5
    or      eax, edx
    and     r8d, 0ffh
    shr     r8d, 3
    or      eax, r8d
    ret                                 ; result in ax
AsmRGB565 endp

; --------------------------------------------------------------------------
; void AsmRGB555To565(int w, int h, void* bmp)       ; ecx/edx/r8
; --------------------------------------------------------------------------
public AsmRGB555To565
AsmRGB555To565 proc
    mov     r9d, edx                    ; row counter
row_555to565:
    mov     r10d, ecx                   ; column counter
col_555to565:
    mov     ax, word ptr [r8]
    mov     r11w, ax
    shr     ax, 5
    shl     ax, 6
    and     r11w, 001fh
    or      ax, r11w
    mov     word ptr [r8], ax
    add     r8, 2
    dec     r10d
    jnz     col_555to565
    dec     r9d
    jnz     row_555to565
    ret
AsmRGB555To565 endp

; --------------------------------------------------------------------------
; void AsmRGB565To555(int w, int h, void* bmp)       ; ecx/edx/r8
; --------------------------------------------------------------------------
public AsmRGB565To555
AsmRGB565To555 proc
    mov     r9d, edx                    ; row counter
row_565to555:
    mov     r10d, ecx                   ; column counter
col_565to555:
    mov     ax, word ptr [r8]
    mov     r11w, ax
    shr     ax, 6
    shl     ax, 5
    and     r11w, 001fh
    or      ax, r11w
    mov     word ptr [r8], ax
    add     r8, 2
    dec     r10d
    jnz     col_565to555
    dec     r9d
    jnz     row_565to555
    ret
AsmRGB565To555 endp

end

; ==========================================================================
; Engine64_msvc.asm - Engine pixel/color/bitmap routines, Windows x64 (MS ABI)
; Assembled by ml64; GCC/Clang version: Engine64_gnu.S
; See Engine/AsmRoutines.h for the argument contract.
; ==========================================================================

.code

; --------------------------------------------------------------------------
; void DDrawPixel16Core(const DPixelArgs* args)        ; rcx = args
; --------------------------------------------------------------------------
public DDrawPixel16Core
DDrawPixel16Core proc
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
DDrawPixel16Core endp

; --------------------------------------------------------------------------
; void DDrawPixelAlpha16Core(const DPixelArgs* args)   ; rcx = args
; blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f dual-word expansion
; --------------------------------------------------------------------------
public DDrawPixelAlpha16Core
DDrawPixelAlpha16Core proc
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
DDrawPixelAlpha16Core endp

; --------------------------------------------------------------------------
; unsigned short DRGB555Core(int red, int green, int blue)   ; ecx/edx/r8d
; --------------------------------------------------------------------------
public DRGB555Core
DRGB555Core proc
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
DRGB555Core endp

; --------------------------------------------------------------------------
; unsigned short DRGB565Core(int red, int green, int blue)   ; ecx/edx/r8d
; --------------------------------------------------------------------------
public DRGB565Core
DRGB565Core proc
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
DRGB565Core endp

; --------------------------------------------------------------------------
; void DRGB555To565Core(int width, int height, void* bmPtr)  ; ecx/edx/r8
; --------------------------------------------------------------------------
public DRGB555To565Core
DRGB555To565Core proc
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
DRGB555To565Core endp

; --------------------------------------------------------------------------
; void DRGB565To555Core(int width, int height, void* bmPtr)  ; ecx/edx/r8
; --------------------------------------------------------------------------
public DRGB565To555Core
DRGB565To555Core proc
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
DRGB565To555Core endp

; --------------------------------------------------------------------------
; void DDrawBitmapCore(const DBitmapArgs* args)    ; rcx = args
; 8bpp -> 16bpp blit through the palette.
;   rdi = dst, rsi = src, rbx = palette
;   r12d = width, r13d = height, r10 = canvasOffs, r11 = bitmapOffs
; --------------------------------------------------------------------------
public DDrawBitmapCore
DDrawBitmapCore proc
    push    rbx
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, rcx                     ; args

    ; dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      ; y
    mul     dword ptr [r9+8]            ; y * pitch
    mov     r10d, dword ptr [r9+12]     ; x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         ; frameBuffer
    add     rdi, rax

    ; src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    mov     eax, dword ptr [r9+40]      ; srcY
    mul     dword ptr [r9+28]           ; srcY * bmpWidth
    add     eax, dword ptr [r9+36]      ; + srcX
    mov     rsi, qword ptr [r9+48]      ; bitmap
    add     rsi, rax

    mov     rbx, qword ptr [r9+56]      ; palette
    mov     r12d, dword ptr [r9+20]     ; width
    mov     r13d, dword ptr [r9+24]     ; height

    mov     r10d, dword ptr [r9+8]      ; pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   ; canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d                  ; bitmapOffs = bmpWidth - width

drawbitmap_row:
    mov     ecx, r12d
drawbitmap_col:
    movzx   edx, byte ptr [rsi]
    inc     rsi
    mov     ax, word ptr [rbx + rdx*2]
    mov     word ptr [rdi], ax
    add     rdi, 2
    dec     ecx
    jnz     drawbitmap_col
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     drawbitmap_row

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    ret
DDrawBitmapCore endp

; --------------------------------------------------------------------------
; void DDrawBitmapTransCore(const DBitmapArgs* args)   ; rcx = args
; 8bpp -> 16bpp blit, 50% blend with the canvas via 0x07e0f81f
; dual-word expansion: blended = (src + dst) / 2 per channel.
; Same register layout as DDrawBitmapCore.
; --------------------------------------------------------------------------
public DDrawBitmapTransCore
DDrawBitmapTransCore proc
    push    rbx
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, rcx                     ; args

    ; dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      ; y
    mul     dword ptr [r9+8]            ; y * pitch
    mov     r10d, dword ptr [r9+12]     ; x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         ; frameBuffer
    add     rdi, rax

    ; src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    mov     eax, dword ptr [r9+40]      ; srcY
    mul     dword ptr [r9+28]           ; srcY * bmpWidth
    add     eax, dword ptr [r9+36]      ; + srcX
    mov     rsi, qword ptr [r9+48]      ; bitmap
    add     rsi, rax

    mov     rbx, qword ptr [r9+56]      ; palette
    mov     r12d, dword ptr [r9+20]     ; width
    mov     r13d, dword ptr [r9+24]     ; height

    mov     r10d, dword ptr [r9+8]      ; pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   ; canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d                  ; bitmapOffs = bmpWidth - width

drawbitmaptrans_row:
    mov     ecx, r12d
drawbitmaptrans_col:
    movzx   eax, byte ptr [rsi]
    inc     rsi
    movzx   edx, word ptr [rbx + rax*2] ; src color
    mov     eax, edx
    shl     eax, 16
    mov     ax, dx
    and     eax, 07e0f81fh              ; src expanded: g high, r|b low
    movzx   edx, word ptr [rdi]         ; dst color
    mov     r8d, edx
    shl     r8d, 16
    mov     r8w, dx
    and     r8d, 07e0f81fh              ; dst expanded
    add     eax, r8d
    shr     eax, 1
    and     eax, 07e0f81fh
    mov     dx, ax
    shr     eax, 16
    or      ax, dx
    mov     word ptr [rdi], ax
    add     rdi, 2
    dec     ecx
    jnz     drawbitmaptrans_col
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     drawbitmaptrans_row

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    ret
DDrawBitmapTransCore endp

; --------------------------------------------------------------------------
; void DDrawBitmap16Core(const DBitmapArgs* args)  ; rcx = args
; 16bpp copy.
;   rdi = dst, rsi = src
;   r12d = width, r13d = height, r10 = canvasOffs, r11 = bitmapOffs
; --------------------------------------------------------------------------
public DDrawBitmap16Core
DDrawBitmap16Core proc
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, rcx                     ; args

    ; dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      ; y
    mul     dword ptr [r9+8]            ; y * pitch
    mov     r10d, dword ptr [r9+12]     ; x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         ; frameBuffer
    add     rdi, rax

    ; src = bitmap + (srcY*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+40]      ; srcY
    mul     dword ptr [r9+28]           ; srcY * bmpWidth
    add     eax, dword ptr [r9+36]      ; + srcX
    add     eax, eax                    ; 16-bit pixels
    mov     rsi, qword ptr [r9+48]      ; bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     ; width
    mov     r13d, dword ptr [r9+24]     ; height

    mov     r10d, dword ptr [r9+8]      ; pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   ; canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d
    add     r11d, r11d                  ; bitmapOffs = (bmpWidth - width) * 2

drawbitmap16_row:
    mov     ecx, r12d
    rep movsw
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     drawbitmap16_row

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret
DDrawBitmap16Core endp

; --------------------------------------------------------------------------
; void DDrawBitmap16MmxCore(const DBitmapArgs* args)   ; rcx = args
; 16bpp copy, 8 bytes per movq with a word tail.
; Same register layout as DDrawBitmap16Core.
; --------------------------------------------------------------------------
public DDrawBitmap16MmxCore
DDrawBitmap16MmxCore proc
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, rcx                     ; args

    ; dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      ; y
    mul     dword ptr [r9+8]            ; y * pitch
    mov     r10d, dword ptr [r9+12]     ; x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         ; frameBuffer
    add     rdi, rax

    ; src = bitmap + (srcY*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+40]      ; srcY
    mul     dword ptr [r9+28]           ; srcY * bmpWidth
    add     eax, dword ptr [r9+36]      ; + srcX
    add     eax, eax                    ; 16-bit pixels
    mov     rsi, qword ptr [r9+48]      ; bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     ; width
    mov     r13d, dword ptr [r9+24]     ; height

    mov     r10d, dword ptr [r9+8]      ; pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   ; canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d
    add     r11d, r11d                  ; bitmapOffs = (bmpWidth - width) * 2

drawbitmap16mmx_row:
    mov     ecx, r12d
    shr     ecx, 2                      ; 4 pixels per movq
    jz      drawbitmap16mmx_tail
drawbitmap16mmx_quad:
    movq    mm0, qword ptr [rsi]
    movq    qword ptr [rdi], mm0
    add     rsi, 8
    add     rdi, 8
    dec     ecx
    jnz     drawbitmap16mmx_quad
drawbitmap16mmx_tail:
    mov     ecx, r12d
    and     ecx, 3
    rep movsw
    add     rsi, r11
    add     rdi, r10
    dec     r13d
    jnz     drawbitmap16mmx_row
    emms

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret
DDrawBitmap16MmxCore endp

; --------------------------------------------------------------------------
; void DDrawBitmap16RevCore(const DBitmapArgs* args)   ; rcx = args
; 16bpp copy, vertically flipped: the first canvas row reads the
; bitmap's last row.
; Same register layout as DDrawBitmap16Core; r11 = bytes to step the
; source back per row.
; --------------------------------------------------------------------------
public DDrawBitmap16RevCore
DDrawBitmap16RevCore proc
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, rcx                     ; args

    ; dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      ; y
    mul     dword ptr [r9+8]            ; y * pitch
    mov     r10d, dword ptr [r9+12]     ; x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         ; frameBuffer
    add     rdi, rax

    ; src = bitmap + ((bmpHeight-1-srcY)*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+32]      ; bmpHeight
    dec     eax
    sub     eax, dword ptr [r9+40]      ; bmpHeight - 1 - srcY
    mul     dword ptr [r9+28]           ; * bmpWidth
    add     eax, dword ptr [r9+36]      ; + srcX
    add     eax, eax                    ; 16-bit pixels
    mov     rsi, qword ptr [r9+48]      ; bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     ; width
    mov     r13d, dword ptr [r9+24]     ; height

    mov     r10d, dword ptr [r9+8]      ; pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   ; canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    add     r11d, r12d
    add     r11d, r11d                  ; rowBack = (bmpWidth + width) * 2

drawbitmap16rev_row:
    mov     ecx, r12d
    shr     ecx, 2                      ; 4 pixels per movq
    jz      drawbitmap16rev_tail
drawbitmap16rev_quad:
    movq    mm0, qword ptr [rsi]
    movq    qword ptr [rdi], mm0
    add     rsi, 8
    add     rdi, 8
    dec     ecx
    jnz     drawbitmap16rev_quad
drawbitmap16rev_tail:
    mov     ecx, r12d
    and     ecx, 3
    rep movsw
    sub     rsi, r11
    add     rdi, r10
    dec     r13d
    jnz     drawbitmap16rev_row
    emms

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret
DDrawBitmap16RevCore endp

end

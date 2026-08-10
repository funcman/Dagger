// ==========================================================================
// Engine64_gnu.S - Engine pixel/color/bitmap routines, x86-64, GCC/Clang (GAS)
// MSVC (ml64) version: Engine64_msvc.asm; see Engine/AsmRoutines.h for
// the argument contract.
// Linux/macOS use the System V ABI; MinGW/clang-gnu use the MS ABI under
// _WIN64. Only the argument registers differ; the bodies are shared via
// the macros below.
// ==========================================================================
    .intel_syntax noprefix
    .text

#if defined(_WIN64)
#   define APTR  rcx        // 1st argument (struct pointer / bitmap pointer)
#   define ARG1d ecx        // 1st/2nd/3rd of the 3-integer-argument routines
#   define ARG2d edx
#   define ARG3  r8
#   define ARG3d r8d
#else
#   define APTR  rdi
#   define ARG1d edi
#   define ARG2d esi
#   define ARG3  rdx
#   define ARG3d edx
#endif

#if defined(_WIN64)
// On COFF targets exported symbols must be explicitly marked as functions
// (.type 32); otherwise they are exported as data, callers get 32-bit
// pseudo-relocations, and the process crashes at runtime when ASLR loads
// the DLL above the 4GB mark.
    .def DDrawPixel16Core;      .scl 2; .type 32; .endef
    .def DDrawPixelAlpha16Core; .scl 2; .type 32; .endef
    .def DRGB555Core;           .scl 2; .type 32; .endef
    .def DRGB565Core;           .scl 2; .type 32; .endef
    .def DRGB555To565Core;      .scl 2; .type 32; .endef
    .def DRGB565To555Core;      .scl 2; .type 32; .endef
    .def DDrawBitmapCore;       .scl 2; .type 32; .endef
    .def DDrawBitmapTransCore;  .scl 2; .type 32; .endef
    .def DDrawBitmap16Core;     .scl 2; .type 32; .endef
    .def DDrawBitmap16MmxCore;  .scl 2; .type 32; .endef
    .def DDrawBitmap16RevCore;  .scl 2; .type 32; .endef
#endif

// --------------------------------------------------------------------------
// void DDrawPixel16Core(const DPixelArgs* args)
// --------------------------------------------------------------------------
    .globl DDrawPixel16Core
DDrawPixel16Core:
    mov     eax, dword ptr [APTR+16]    // y
    mul     dword ptr [APTR+8]          // y * pitch
    mov     r10d, dword ptr [APTR+12]   // x
    add     eax, r10d
    add     eax, r10d                   // offset = y*pitch + 2*x
    mov     r11, qword ptr [APTR]       // frameBuffer
    add     r11, rax
    mov     ax, word ptr [APTR+20]      // color
    mov     word ptr [r11], ax
    ret

// --------------------------------------------------------------------------
// void DDrawPixelAlpha16Core(const DPixelArgs* args)
// blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f dual-word expansion
// --------------------------------------------------------------------------
    .globl DDrawPixelAlpha16Core
DDrawPixelAlpha16Core:
    mov     eax, dword ptr [APTR+16]    // y
    mul     dword ptr [APTR+8]          // y * pitch
    mov     r10d, dword ptr [APTR+12]   // x
    add     eax, r10d
    add     eax, r10d
    mov     r11, qword ptr [APTR]
    add     r11, rax                    // r11 = target pixel address

    mov     eax, dword ptr [APTR+20]    // color
    mov     r10d, eax
    shl     eax, 16
    mov     ax, r10w
    and     eax, 0x07e0f81f             // src expanded

    movzx   r10d, word ptr [r11]        // dst
    mov     r8d, r10d
    shl     r10d, 16
    mov     r10w, r8w
    and     r10d, 0x07e0f81f            // dst expanded

    mov     r8d, dword ptr [APTR+24]    // alpha
    mul     r8d                         // src * alpha
    neg     r8d
    add     r8d, 32                     // 32 - alpha
    xchg    eax, r10d
    mul     r8d                         // dst * (32 - alpha)
    add     eax, r10d
    shr     eax, 5
    and     eax, 0x07e0f81f
    mov     r10w, ax
    shr     eax, 16
    or      ax, r10w
    mov     word ptr [r11], ax
    ret

// --------------------------------------------------------------------------
// unsigned short DRGB555Core(int red, int green, int blue)
// --------------------------------------------------------------------------
    .globl DRGB555Core
DRGB555Core:
    mov     eax, ARG1d
    and     eax, 0xff
    shr     eax, 3
    shl     eax, 10
    and     ARG2d, 0xff
    shr     ARG2d, 3
    shl     ARG2d, 5
    or      eax, ARG2d
    and     ARG3d, 0xff
    shr     ARG3d, 3
    or      eax, ARG3d
    ret                                 // result in ax

// --------------------------------------------------------------------------
// unsigned short DRGB565Core(int red, int green, int blue)
// --------------------------------------------------------------------------
    .globl DRGB565Core
DRGB565Core:
    mov     eax, ARG1d
    and     eax, 0xff
    shr     eax, 3
    shl     eax, 11
    and     ARG2d, 0xff
    shr     ARG2d, 2
    shl     ARG2d, 5
    or      eax, ARG2d
    and     ARG3d, 0xff
    shr     ARG3d, 3
    or      eax, ARG3d
    ret                                 // result in ax

// --------------------------------------------------------------------------
// void DRGB555To565Core(int width, int height, void* bmPtr)
// --------------------------------------------------------------------------
    .globl DRGB555To565Core
DRGB555To565Core:
    mov     r9d, ARG2d                  // row counter
.Lrow_555to565:
    mov     r10d, ARG1d                 // column counter
.Lcol_555to565:
    mov     ax, word ptr [ARG3]
    mov     r11w, ax
    shr     ax, 5
    shl     ax, 6
    and     r11w, 0x001f
    or      ax, r11w
    mov     word ptr [ARG3], ax
    add     ARG3, 2
    dec     r10d
    jnz     .Lcol_555to565
    dec     r9d
    jnz     .Lrow_555to565
    ret

// --------------------------------------------------------------------------
// void DRGB565To555Core(int width, int height, void* bmPtr)
// --------------------------------------------------------------------------
    .globl DRGB565To555Core
DRGB565To555Core:
    mov     r9d, ARG2d                  // row counter
.Lrow_565to555:
    mov     r10d, ARG1d                 // column counter
.Lcol_565to555:
    mov     ax, word ptr [ARG3]
    mov     r11w, ax
    shr     ax, 6
    shl     ax, 5
    and     r11w, 0x001f
    or      ax, r11w
    mov     word ptr [ARG3], ax
    add     ARG3, 2
    dec     r10d
    jnz     .Lcol_565to555
    dec     r9d
    jnz     .Lrow_565to555
    ret

// --------------------------------------------------------------------------
// void DDrawBitmapCore(const DBitmapArgs* args)
// 8bpp -> 16bpp blit through the palette.
//   rdi = dst, rsi = src, rbx = palette
//   r12d = width, r13d = height, r10 = canvasOffs, r11 = bitmapOffs
// --------------------------------------------------------------------------
    .globl DDrawBitmapCore
DDrawBitmapCore:
    push    rbx
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, APTR                    // args

    // dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      // y
    mul     dword ptr [r9+8]            // y * pitch
    mov     r10d, dword ptr [r9+12]     // x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         // frameBuffer
    add     rdi, rax

    // src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    mov     eax, dword ptr [r9+40]      // srcY
    mul     dword ptr [r9+28]           // srcY * bmpWidth
    add     eax, dword ptr [r9+36]      // + srcX
    mov     rsi, qword ptr [r9+48]      // bitmap
    add     rsi, rax

    mov     rbx, qword ptr [r9+56]      // palette
    mov     r12d, dword ptr [r9+20]     // width
    mov     r13d, dword ptr [r9+24]     // height

    mov     r10d, dword ptr [r9+8]      // pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   // canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d                  // bitmapOffs = bmpWidth - width

.Lrow_drawbitmap:
    mov     ecx, r12d
.Lcol_drawbitmap:
    movzx   edx, byte ptr [rsi]
    inc     rsi
    mov     ax, word ptr [rbx + rdx*2]
    mov     word ptr [rdi], ax
    add     rdi, 2
    dec     ecx
    jnz     .Lcol_drawbitmap
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     .Lrow_drawbitmap

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    ret

// --------------------------------------------------------------------------
// void DDrawBitmapTransCore(const DBitmapArgs* args)
// 8bpp -> 16bpp blit, 50% blend with the canvas via 0x07e0f81f
// dual-word expansion: blended = (src + dst) / 2 per channel.
// Same register layout as DDrawBitmapCore.
// --------------------------------------------------------------------------
    .globl DDrawBitmapTransCore
DDrawBitmapTransCore:
    push    rbx
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, APTR                    // args

    // dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      // y
    mul     dword ptr [r9+8]            // y * pitch
    mov     r10d, dword ptr [r9+12]     // x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         // frameBuffer
    add     rdi, rax

    // src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    mov     eax, dword ptr [r9+40]      // srcY
    mul     dword ptr [r9+28]           // srcY * bmpWidth
    add     eax, dword ptr [r9+36]      // + srcX
    mov     rsi, qword ptr [r9+48]      // bitmap
    add     rsi, rax

    mov     rbx, qword ptr [r9+56]      // palette
    mov     r12d, dword ptr [r9+20]     // width
    mov     r13d, dword ptr [r9+24]     // height

    mov     r10d, dword ptr [r9+8]      // pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   // canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d                  // bitmapOffs = bmpWidth - width

.Lrow_drawbitmaptrans:
    mov     ecx, r12d
.Lcol_drawbitmaptrans:
    movzx   eax, byte ptr [rsi]
    inc     rsi
    movzx   edx, word ptr [rbx + rax*2] // src color
    mov     eax, edx
    shl     eax, 16
    mov     ax, dx
    and     eax, 0x07e0f81f             // src expanded: g high, r|b low
    movzx   edx, word ptr [rdi]         // dst color
    mov     r8d, edx
    shl     r8d, 16
    mov     r8w, dx
    and     r8d, 0x07e0f81f             // dst expanded
    add     eax, r8d
    shr     eax, 1
    and     eax, 0x07e0f81f
    mov     dx, ax
    shr     eax, 16
    or      ax, dx
    mov     word ptr [rdi], ax
    add     rdi, 2
    dec     ecx
    jnz     .Lcol_drawbitmaptrans
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     .Lrow_drawbitmaptrans

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    pop     rbx
    ret

// --------------------------------------------------------------------------
// void DDrawBitmap16Core(const DBitmapArgs* args)
// 16bpp copy.
//   rdi = dst, rsi = src
//   r12d = width, r13d = height, r10 = canvasOffs, r11 = bitmapOffs
// --------------------------------------------------------------------------
    .globl DDrawBitmap16Core
DDrawBitmap16Core:
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, APTR                    // args

    // dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      // y
    mul     dword ptr [r9+8]            // y * pitch
    mov     r10d, dword ptr [r9+12]     // x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         // frameBuffer
    add     rdi, rax

    // src = bitmap + (srcY*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+40]      // srcY
    mul     dword ptr [r9+28]           // srcY * bmpWidth
    add     eax, dword ptr [r9+36]      // + srcX
    add     eax, eax                    // 16-bit pixels
    mov     rsi, qword ptr [r9+48]      // bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     // width
    mov     r13d, dword ptr [r9+24]     // height

    mov     r10d, dword ptr [r9+8]      // pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   // canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d
    add     r11d, r11d                  // bitmapOffs = (bmpWidth - width) * 2

.Lrow_drawbitmap16:
    mov     ecx, r12d
    rep movsw
    add     rdi, r10
    add     rsi, r11
    dec     r13d
    jnz     .Lrow_drawbitmap16

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret

// --------------------------------------------------------------------------
// void DDrawBitmap16MmxCore(const DBitmapArgs* args)
// 16bpp copy, 8 bytes per movq with a word tail.
// Same register layout as DDrawBitmap16Core.
// --------------------------------------------------------------------------
    .globl DDrawBitmap16MmxCore
DDrawBitmap16MmxCore:
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, APTR                    // args

    // dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      // y
    mul     dword ptr [r9+8]            // y * pitch
    mov     r10d, dword ptr [r9+12]     // x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         // frameBuffer
    add     rdi, rax

    // src = bitmap + (srcY*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+40]      // srcY
    mul     dword ptr [r9+28]           // srcY * bmpWidth
    add     eax, dword ptr [r9+36]      // + srcX
    add     eax, eax                    // 16-bit pixels
    mov     rsi, qword ptr [r9+48]      // bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     // width
    mov     r13d, dword ptr [r9+24]     // height

    mov     r10d, dword ptr [r9+8]      // pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   // canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    sub     r11d, r12d
    add     r11d, r11d                  // bitmapOffs = (bmpWidth - width) * 2

.Lrow_drawbitmap16mmx:
    mov     ecx, r12d
    shr     ecx, 2                      // 4 pixels per movq
    jz      .Ltail_drawbitmap16mmx
.Lquad_drawbitmap16mmx:
    movq    mm0, qword ptr [rsi]
    movq    qword ptr [rdi], mm0
    add     rsi, 8
    add     rdi, 8
    dec     ecx
    jnz     .Lquad_drawbitmap16mmx
.Ltail_drawbitmap16mmx:
    mov     ecx, r12d
    and     ecx, 3
    rep movsw
    add     rsi, r11
    add     rdi, r10
    dec     r13d
    jnz     .Lrow_drawbitmap16mmx
    emms

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret

// --------------------------------------------------------------------------
// void DDrawBitmap16RevCore(const DBitmapArgs* args)
// 16bpp copy, vertically flipped: the first canvas row reads the
// bitmap's last row.
// Same register layout as DDrawBitmap16Core; r11 = bytes to step the
// source back per row.
// --------------------------------------------------------------------------
    .globl DDrawBitmap16RevCore
DDrawBitmap16RevCore:
    push    rsi
    push    rdi
    push    r12
    push    r13
    mov     r9, APTR                    // args

    // dst = frameBuffer + y*pitch + 2*x
    mov     eax, dword ptr [r9+16]      // y
    mul     dword ptr [r9+8]            // y * pitch
    mov     r10d, dword ptr [r9+12]     // x
    add     eax, r10d
    add     eax, r10d
    mov     rdi, qword ptr [r9]         // frameBuffer
    add     rdi, rax

    // src = bitmap + ((bmpHeight-1-srcY)*bmpWidth + srcX) * 2
    mov     eax, dword ptr [r9+32]      // bmpHeight
    dec     eax
    sub     eax, dword ptr [r9+40]      // bmpHeight - 1 - srcY
    mul     dword ptr [r9+28]           // * bmpWidth
    add     eax, dword ptr [r9+36]      // + srcX
    add     eax, eax                    // 16-bit pixels
    mov     rsi, qword ptr [r9+48]      // bitmap
    add     rsi, rax

    mov     r12d, dword ptr [r9+20]     // width
    mov     r13d, dword ptr [r9+24]     // height

    mov     r10d, dword ptr [r9+8]      // pitch
    mov     eax, r12d
    add     eax, eax
    sub     r10d, eax                   // canvasOffs = pitch - width*2
    mov     r11d, dword ptr [r9+28]
    add     r11d, r12d
    add     r11d, r11d                  // rowBack = (bmpWidth + width) * 2

.Lrow_drawbitmap16rev:
    mov     ecx, r12d
    shr     ecx, 2                      // 4 pixels per movq
    jz      .Ltail_drawbitmap16rev
.Lquad_drawbitmap16rev:
    movq    mm0, qword ptr [rsi]
    movq    qword ptr [rdi], mm0
    add     rsi, 8
    add     rdi, 8
    dec     ecx
    jnz     .Lquad_drawbitmap16rev
.Ltail_drawbitmap16rev:
    mov     ecx, r12d
    and     ecx, 3
    rep movsw
    sub     rsi, r11
    add     rdi, r10
    dec     r13d
    jnz     .Lrow_drawbitmap16rev
    emms

    pop     r13
    pop     r12
    pop     rdi
    pop     rsi
    ret

#if !defined(_WIN64)
    .section .note.GNU-stack,"",@progbits
#endif

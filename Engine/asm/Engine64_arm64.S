// ==========================================================================
// Engine64_arm64.S - Engine pixel/color/bitmap routines, ARM64 (AArch64), GAS
//
// Calling convention: System V / AAPCS64
//   x0-x7 : arguments (x0 also used for return value)
//   x30   : link register
//
// Optimizations over the first ARM64 pass:
//   - DDrawPixel16Core / DDrawPixelAlpha16Core use a single madd for address
//     calculation and store with shifted offset.
//   - DDrawPixelAlpha16Core uses ORR-with-shift for color expansion and madd
//     to fold the final multiply-add.
//   - DRGB555Core / DRGB565Core use UBFX + shifted ORR.
//   - DRGB555To565Core / DRGB565To555Core use NEON to process 8 pixels/iter
//     with a scalar tail loop.
//
// macOS requires an underscore prefix on C symbols; Linux ELF does not.
// ==========================================================================

#if defined(__APPLE__)
#   define SYMBOL(name) _##name
#else
#   define SYMBOL(name) name
#endif

    .text
    .p2align 2

// --------------------------------------------------------------------------
// void DDrawPixel16Core(const DPixelArgs* args)
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawPixel16Core)
SYMBOL(DDrawPixel16Core):
    ldr     x3, [x0]                // frameBuffer
    ldrsw   x4, [x0, #16]           // y (sign-extended)
    ldrsw   x5, [x0, #8]            // pitch (sign-extended)
    madd    x4, x4, x5, x3          // frameBuffer + y * pitch
    ldrsw   x5, [x0, #12]           // x (sign-extended)
    ldrh    w3, [x0, #20]           // color
    strh    w3, [x4, x5, lsl #1]    // *(frameBuffer + y*pitch + 2*x)
    ret

// --------------------------------------------------------------------------
// void DDrawPixelAlpha16Core(const DPixelArgs* args)
// blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f expansion
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawPixelAlpha16Core)
SYMBOL(DDrawPixelAlpha16Core):
    // Pixel address
    ldr     x3, [x0]                // frameBuffer
    ldrsw   x4, [x0, #16]           // y
    ldrsw   x5, [x0, #8]            // pitch
    madd    x4, x4, x5, x3          // frameBuffer + y*pitch
    ldrsw   x5, [x0, #12]           // x
    add     x4, x4, x5, lsl #1      // x4 = target pixel address

    // Mask 0x07e0f81f
    mov     w6, #0xf81f
    movk    w6, #0x07e0, lsl #16

    // Source color expansion: ((color << 16) | color) & mask
    ldr     w3, [x0, #20]           // color (low 16 bits used)
    orr     w3, w3, w3, lsl #16
    and     w3, w3, w6              // w3 = src expanded

    // Destination color expansion
    ldrh    w5, [x4]
    orr     w5, w5, w5, lsl #16
    and     w5, w5, w6              // w5 = dst expanded

    // Blend: src*alpha + dst*(32-alpha)
    ldr     w7, [x0, #24]           // alpha
    mul     w3, w3, w7              // src * alpha
    neg     w7, w7
    add     w7, w7, #32             // 32 - alpha
    madd    w3, w5, w7, w3          // w3 = src*alpha + dst*(32-alpha)
    lsr     w3, w3, #5
    and     w3, w3, w6

    // Recombine: green (high 16) | red+blue (low 16)
    mov     w5, w3                  // w5 = red+blue
    lsr     w3, w3, #16             // w3 = green
    orr     w3, w3, w5
    strh    w3, [x4]
    ret

// --------------------------------------------------------------------------
// unsigned short DRGB555Core(int red, int green, int blue)
// --------------------------------------------------------------------------
    .globl SYMBOL(DRGB555Core)
SYMBOL(DRGB555Core):
    ubfx    w0, w0, #3, #5          // r in bits 0-4
    lsl     w0, w0, #10             // r in bits 10-14
    ubfx    w1, w1, #3, #5          // g in bits 0-4
    orr     w0, w0, w1, lsl #5      // g in bits 5-9
    ubfx    w2, w2, #3, #5          // b in bits 0-4
    orr     w0, w0, w2              // b in bits 0-4
    ret

// --------------------------------------------------------------------------
// unsigned short DRGB565Core(int red, int green, int blue)
// --------------------------------------------------------------------------
    .globl SYMBOL(DRGB565Core)
SYMBOL(DRGB565Core):
    ubfx    w0, w0, #3, #5          // r in bits 0-4
    lsl     w0, w0, #11             // r in bits 11-15
    ubfx    w1, w1, #2, #6          // g in bits 0-5
    orr     w0, w0, w1, lsl #5      // g in bits 5-10
    ubfx    w2, w2, #3, #5          // b in bits 0-4
    orr     w0, w0, w2              // b in bits 0-4
    ret

// --------------------------------------------------------------------------
// void DRGB555To565Core(int width, int height, void* bmPtr)
// NEON inner loop: 8 pixels/iter, scalar tail.
// --------------------------------------------------------------------------
    .globl SYMBOL(DRGB555To565Core)
SYMBOL(DRGB555To565Core):
    // Prepare NEON masks once.
    mov     w11, #0x7fe0
    dup     v1.8h, w11              // v1 = 0x7fe0 x 8
    movi    v2.8h, #0x1f            // v2 = 0x001f x 8

    mov     w9, w1                  // row counter
.Lrow_555to565:
    mov     w10, w0                 // column counter
.Lcol_555to565_vec:
    cmp     w10, #8
    b.lt    .Lcol_555to565_scalar
    sub     w10, w10, #8
    ld1     {v0.8h}, [x2]
    and     v3.16b, v0.16b, v1.16b
    shl     v3.8h, v3.8h, #1
    and     v0.16b, v0.16b, v2.16b
    orr     v0.16b, v0.16b, v3.16b
    st1     {v0.8h}, [x2], #16
    b       .Lcol_555to565_vec
.Lcol_555to565_scalar:
    cbz     w10, .Lnext_row_555to565
    ldrh    w3, [x2]
    and     w4, w3, #0x7fe0
    lsl     w4, w4, #1
    and     w3, w3, #0x001f
    orr     w3, w4, w3
    strh    w3, [x2], #2
    sub     w10, w10, #1
    b       .Lcol_555to565_scalar
.Lnext_row_555to565:
    subs    w9, w9, #1
    b.ne    .Lrow_555to565
    ret

// --------------------------------------------------------------------------
// void DRGB565To555Core(int width, int height, void* bmPtr)
// NEON inner loop: 8 pixels/iter, scalar tail.
// --------------------------------------------------------------------------
    .globl SYMBOL(DRGB565To555Core)
SYMBOL(DRGB565To555Core):
    // Prepare NEON masks once.
    mov     w11, #0xffc0
    dup     v1.8h, w11              // v1 = 0xffc0 x 8
    movi    v2.8h, #0x1f            // v2 = 0x001f x 8

    mov     w9, w1                  // row counter
.Lrow_565to555:
    mov     w10, w0                 // column counter
.Lcol_565to555_vec:
    cmp     w10, #8
    b.lt    .Lcol_565to555_scalar
    sub     w10, w10, #8
    ld1     {v0.8h}, [x2]
    and     v3.16b, v0.16b, v1.16b
    ushr    v3.8h, v3.8h, #1
    and     v0.16b, v0.16b, v2.16b
    orr     v0.16b, v0.16b, v3.16b
    st1     {v0.8h}, [x2], #16
    b       .Lcol_565to555_vec
.Lcol_565to555_scalar:
    cbz     w10, .Lnext_row_565to555
    ldrh    w3, [x2]
    and     w4, w3, #0xffc0
    lsr     w4, w4, #1
    and     w3, w3, #0x001f
    orr     w3, w4, w3
    strh    w3, [x2], #2
    sub     w10, w10, #1
    b       .Lcol_565to555_scalar
.Lnext_row_565to555:
    subs    w9, w9, #1
    b.ne    .Lrow_565to555
    ret

// --------------------------------------------------------------------------
// Bitmap blitters. Shared register layout:
//   x9  = dst, x10 = src, x11 = palette (8bpp only)
//   w12 = width, x13 = height, x14 = canvasOffs, x15 = bitmapOffs
//   w2 = column counter, w3/w4 = scratch
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// void DDrawBitmapCore(const DBitmapArgs* args)
// 8bpp -> 16bpp blit through the palette.
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawBitmapCore)
SYMBOL(DDrawBitmapCore):
    // dst = frameBuffer + y*pitch + 2*x
    ldr     x9, [x0]                // frameBuffer
    ldrsw   x10, [x0, #16]          // y
    ldrsw   x11, [x0, #8]           // pitch
    madd    x9, x10, x11, x9        // frameBuffer + y*pitch
    ldrsw   x10, [x0, #12]          // x
    add     x9, x9, x10, lsl #1     // x9 = dst

    // src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    ldrsw   x10, [x0, #40]          // srcY
    ldrsw   x11, [x0, #28]          // bmpWidth
    mul     x10, x10, x11
    ldrsw   x11, [x0, #36]          // srcX
    add     x10, x10, x11
    ldr     x11, [x0, #48]          // bitmap
    add     x10, x11, x10           // x10 = src

    ldr     x11, [x0, #56]          // palette
    ldrsw   x12, [x0, #20]          // width
    ldrsw   x13, [x0, #24]          // height
    ldrsw   x14, [x0, #8]
    sub     x14, x14, x12, lsl #1   // canvasOffs = pitch - width*2
    ldrsw   x15, [x0, #28]
    sub     x15, x15, x12           // bitmapOffs = bmpWidth - width

.Lrow_drawbitmap:
    mov     w2, w12                 // column counter
.Lcol_drawbitmap:
    ldrb    w3, [x10], #1
    ldrh    w4, [x11, w3, uxtw #1]
    strh    w4, [x9], #2
    subs    w2, w2, #1
    b.ne    .Lcol_drawbitmap
    add     x9, x9, x14
    add     x10, x10, x15
    subs    x13, x13, #1
    b.ne    .Lrow_drawbitmap
    ret

// --------------------------------------------------------------------------
// void DDrawBitmapTransCore(const DBitmapArgs* args)
// 8bpp -> 16bpp blit, 50% blend with the canvas via 0x07e0f81f
// dual-word expansion: blended = (src + dst) / 2 per channel.
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawBitmapTransCore)
SYMBOL(DDrawBitmapTransCore):
    // dst = frameBuffer + y*pitch + 2*x
    ldr     x9, [x0]                // frameBuffer
    ldrsw   x10, [x0, #16]          // y
    ldrsw   x11, [x0, #8]           // pitch
    madd    x9, x10, x11, x9        // frameBuffer + y*pitch
    ldrsw   x10, [x0, #12]          // x
    add     x9, x9, x10, lsl #1     // x9 = dst

    // src = bitmap + srcY*bmpWidth + srcX  (8-bit indices)
    ldrsw   x10, [x0, #40]          // srcY
    ldrsw   x11, [x0, #28]          // bmpWidth
    mul     x10, x10, x11
    ldrsw   x11, [x0, #36]          // srcX
    add     x10, x10, x11
    ldr     x11, [x0, #48]          // bitmap
    add     x10, x11, x10           // x10 = src

    ldr     x11, [x0, #56]          // palette
    ldrsw   x12, [x0, #20]          // width
    ldrsw   x13, [x0, #24]          // height
    ldrsw   x14, [x0, #8]
    sub     x14, x14, x12, lsl #1   // canvasOffs = pitch - width*2
    ldrsw   x15, [x0, #28]
    sub     x15, x15, x12           // bitmapOffs = bmpWidth - width

    mov     w6, #0xf81f
    movk    w6, #0x07e0, lsl #16    // mask 0x07e0f81f

.Lrow_drawbitmaptrans:
    mov     w2, w12                 // column counter
.Lcol_drawbitmaptrans:
    ldrb    w3, [x10], #1
    ldrh    w3, [x11, w3, uxtw #1]  // src color
    orr     w3, w3, w3, lsl #16
    and     w3, w3, w6              // src expanded: g high, r|b low
    ldrh    w4, [x9]                // dst color
    orr     w4, w4, w4, lsl #16
    and     w4, w4, w6              // dst expanded
    add     w3, w3, w4
    lsr     w3, w3, #1
    and     w3, w3, w6
    mov     w4, w3                  // w4 = r|b
    lsr     w3, w3, #16             // w3 = g
    orr     w3, w3, w4
    strh    w3, [x9], #2
    subs    w2, w2, #1
    b.ne    .Lcol_drawbitmaptrans
    add     x9, x9, x14
    add     x10, x10, x15
    subs    x13, x13, #1
    b.ne    .Lrow_drawbitmaptrans
    ret

// --------------------------------------------------------------------------
// void DDrawBitmap16Core(const DBitmapArgs* args)
// 16bpp copy, 8 bytes per iteration with a word tail.
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawBitmap16Core)
SYMBOL(DDrawBitmap16Core):
    // dst = frameBuffer + y*pitch + 2*x
    ldr     x9, [x0]                // frameBuffer
    ldrsw   x10, [x0, #16]          // y
    ldrsw   x11, [x0, #8]           // pitch
    madd    x9, x10, x11, x9        // frameBuffer + y*pitch
    ldrsw   x10, [x0, #12]          // x
    add     x9, x9, x10, lsl #1     // x9 = dst

    // src = bitmap + (srcY*bmpWidth + srcX) * 2
    ldrsw   x10, [x0, #40]          // srcY
    ldrsw   x11, [x0, #28]          // bmpWidth
    mul     x10, x10, x11
    ldrsw   x11, [x0, #36]          // srcX
    add     x10, x10, x11
    ldr     x11, [x0, #48]          // bitmap
    add     x10, x11, x10, lsl #1   // x10 = src

    ldrsw   x12, [x0, #20]          // width
    ldrsw   x13, [x0, #24]          // height
    ldrsw   x14, [x0, #8]
    sub     x14, x14, x12, lsl #1   // canvasOffs = pitch - width*2
    ldrsw   x15, [x0, #28]
    sub     x15, x15, x12
    lsl     x15, x15, #1            // bitmapOffs = (bmpWidth - width) * 2

.Lrow_drawbitmap16:
    mov     w2, w12                 // column counter
.Lquad_drawbitmap16:
    cmp     w2, #4
    b.lt    .Ltail_drawbitmap16
    ldr     x3, [x10], #8           // 4 pixels
    str     x3, [x9], #8
    sub     w2, w2, #4
    b       .Lquad_drawbitmap16
.Ltail_drawbitmap16:
    cbz     w2, .Lnext_drawbitmap16
    ldrh    w3, [x10], #2
    strh    w3, [x9], #2
    sub     w2, w2, #1
    b       .Ltail_drawbitmap16
.Lnext_drawbitmap16:
    add     x9, x9, x14
    add     x10, x10, x15
    subs    x13, x13, #1
    b.ne    .Lrow_drawbitmap16
    ret

// --------------------------------------------------------------------------
// void DDrawBitmap16MmxCore(const DBitmapArgs* args)
// The MMX variant was only a speed-up of DDrawBitmap16 on x86;
// DDrawBitmap16Core already copies 8 bytes per iteration here.
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawBitmap16MmxCore)
SYMBOL(DDrawBitmap16MmxCore):
    b       SYMBOL(DDrawBitmap16Core)

// --------------------------------------------------------------------------
// void DDrawBitmap16RevCore(const DBitmapArgs* args)
// 16bpp copy, vertically flipped: the first canvas row reads the
// bitmap's last row.
// --------------------------------------------------------------------------
    .globl SYMBOL(DDrawBitmap16RevCore)
SYMBOL(DDrawBitmap16RevCore):
    // dst = frameBuffer + y*pitch + 2*x
    ldr     x9, [x0]                // frameBuffer
    ldrsw   x10, [x0, #16]          // y
    ldrsw   x11, [x0, #8]           // pitch
    madd    x9, x10, x11, x9        // frameBuffer + y*pitch
    ldrsw   x10, [x0, #12]          // x
    add     x9, x9, x10, lsl #1     // x9 = dst

    // src = bitmap + ((bmpHeight-1-srcY)*bmpWidth + srcX) * 2
    ldrsw   x10, [x0, #32]          // bmpHeight
    sub     x10, x10, #1
    ldrsw   x11, [x0, #40]          // srcY
    sub     x10, x10, x11           // bmpHeight - 1 - srcY
    ldrsw   x11, [x0, #28]          // bmpWidth
    mul     x10, x10, x11
    ldrsw   x11, [x0, #36]          // srcX
    add     x10, x10, x11
    ldr     x11, [x0, #48]          // bitmap
    add     x10, x11, x10, lsl #1   // x10 = src

    ldrsw   x12, [x0, #20]          // width
    ldrsw   x13, [x0, #24]          // height
    ldrsw   x14, [x0, #8]
    sub     x14, x14, x12, lsl #1   // canvasOffs = pitch - width*2
    ldrsw   x15, [x0, #28]
    add     x15, x15, x12
    lsl     x15, x15, #1            // rowBack = (bmpWidth + width) * 2

.Lrow_drawbitmap16rev:
    mov     w2, w12                 // column counter
.Lquad_drawbitmap16rev:
    cmp     w2, #4
    b.lt    .Ltail_drawbitmap16rev
    ldr     x3, [x10], #8           // 4 pixels
    str     x3, [x9], #8
    sub     w2, w2, #4
    b       .Lquad_drawbitmap16rev
.Ltail_drawbitmap16rev:
    cbz     w2, .Lnext_drawbitmap16rev
    ldrh    w3, [x10], #2
    strh    w3, [x9], #2
    sub     w2, w2, #1
    b       .Ltail_drawbitmap16rev
.Lnext_drawbitmap16rev:
    add     x9, x9, x14
    sub     x10, x10, x15
    subs    x13, x13, #1
    b.ne    .Lrow_drawbitmap16rev
    ret

#if !defined(__APPLE__)
    .section .note.GNU-stack,"",@progbits
#endif

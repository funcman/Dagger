// ==========================================================================
// Engine64_arm64.S - Engine pixel/color routines, ARM64 (AArch64), GAS
//
// Calling convention: System V / AAPCS64
//   x0-x7 : arguments (x0 also used for return value)
//   x30   : link register
//
// Optimizations over the first ARM64 pass:
//   - AsmDrawPixel16 / AsmDrawPixelAlpha16 use a single madd for address
//     calculation and store with shifted offset.
//   - AsmDrawPixelAlpha16 uses ORR-with-shift for color expansion and madd
//     to fold the final multiply-add.
//   - AsmRGB555 / AsmRGB565 use UBFX + shifted ORR.
//   - AsmRGB555To565 / AsmRGB565To555 use NEON to process 8 pixels/iter
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
// void AsmDrawPixel16(const AsmPixelArgs* args)
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmDrawPixel16)
SYMBOL(AsmDrawPixel16):
    ldr     x3, [x0]                // frameBuffer
    ldrsw   x4, [x0, #16]           // y (sign-extended)
    ldrsw   x5, [x0, #8]            // pitch (sign-extended)
    madd    x4, x4, x5, x3          // frameBuffer + y * pitch
    ldrsw   x5, [x0, #12]           // x (sign-extended)
    ldrh    w3, [x0, #20]           // color
    strh    w3, [x4, x5, lsl #1]    // *(frameBuffer + y*pitch + 2*x)
    ret

// --------------------------------------------------------------------------
// void AsmDrawPixelAlpha16(const AsmPixelArgs* args)
// blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f expansion
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmDrawPixelAlpha16)
SYMBOL(AsmDrawPixelAlpha16):
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
// unsigned short AsmRGB555(int r, int g, int b)
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmRGB555)
SYMBOL(AsmRGB555):
    ubfx    w0, w0, #3, #5          // r in bits 0-4
    lsl     w0, w0, #10             // r in bits 10-14
    ubfx    w1, w1, #3, #5          // g in bits 0-4
    orr     w0, w0, w1, lsl #5      // g in bits 5-9
    ubfx    w2, w2, #3, #5          // b in bits 0-4
    orr     w0, w0, w2              // b in bits 0-4
    ret

// --------------------------------------------------------------------------
// unsigned short AsmRGB565(int r, int g, int b)
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmRGB565)
SYMBOL(AsmRGB565):
    ubfx    w0, w0, #3, #5          // r in bits 0-4
    lsl     w0, w0, #11             // r in bits 11-15
    ubfx    w1, w1, #2, #6          // g in bits 0-5
    orr     w0, w0, w1, lsl #5      // g in bits 5-10
    ubfx    w2, w2, #3, #5          // b in bits 0-4
    orr     w0, w0, w2              // b in bits 0-4
    ret

// --------------------------------------------------------------------------
// void AsmRGB555To565(int w, int h, void* bm_ptr)
// NEON inner loop: 8 pixels/iter, scalar tail.
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmRGB555To565)
SYMBOL(AsmRGB555To565):
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
// void AsmRGB565To555(int w, int h, void* bm_ptr)
// NEON inner loop: 8 pixels/iter, scalar tail.
// --------------------------------------------------------------------------
    .globl SYMBOL(AsmRGB565To555)
SYMBOL(AsmRGB565To555):
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

#if !defined(__APPLE__)
    .section .note.GNU-stack,"",@progbits
#endif

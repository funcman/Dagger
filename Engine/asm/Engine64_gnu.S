// ==========================================================================
// Engine64_gnu.S - Engine pixel/color routines, x86-64, GCC/Clang (GAS)
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
    .def AsmDrawPixel16;      .scl 2; .type 32; .endef
    .def AsmDrawPixelAlpha16; .scl 2; .type 32; .endef
    .def AsmRGB555;           .scl 2; .type 32; .endef
    .def AsmRGB565;           .scl 2; .type 32; .endef
    .def AsmRGB555To565;      .scl 2; .type 32; .endef
    .def AsmRGB565To555;      .scl 2; .type 32; .endef
#endif

// --------------------------------------------------------------------------
// void AsmDrawPixel16(const AsmPixelArgs* args)
// --------------------------------------------------------------------------
    .globl AsmDrawPixel16
AsmDrawPixel16:
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
// void AsmDrawPixelAlpha16(const AsmPixelArgs* args)
// blended = (src*alpha + dst*(32-alpha)) / 32, mixed via 0x07e0f81f dual-word expansion
// --------------------------------------------------------------------------
    .globl AsmDrawPixelAlpha16
AsmDrawPixelAlpha16:
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
// unsigned short AsmRGB555(int r, int g, int b)
// --------------------------------------------------------------------------
    .globl AsmRGB555
AsmRGB555:
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
// unsigned short AsmRGB565(int r, int g, int b)
// --------------------------------------------------------------------------
    .globl AsmRGB565
AsmRGB565:
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
// void AsmRGB555To565(int w, int h, void* bmp)
// --------------------------------------------------------------------------
    .globl AsmRGB555To565
AsmRGB555To565:
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
// void AsmRGB565To555(int w, int h, void* bmp)
// --------------------------------------------------------------------------
    .globl AsmRGB565To555
AsmRGB565To555:
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

#if !defined(_WIN64)
    .section .note.GNU-stack,"",@progbits
#endif

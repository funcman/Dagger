#include "DrawPixel.h"
#include "Base.h"
#include "Canvas.h"

void DrawPixel(int x, int y, int color) {
    void*   frameBuffer = GpCanvas->frameBuffer;
    long    pitch       = GpCanvas->pitch;

    long    clipLeft    = GpCanvas->rect.left;
    long    clipTop     = GpCanvas->rect.top;
    long    clipRight   = GpCanvas->rect.right;
    long    clipBottom  = GpCanvas->rect.bottom;

#ifndef __GNUC__
    __asm {
        mov		eax, y
        cmp		eax, clipTop
        jl		loc_PutPixel_exit
        cmp		eax, clipBottom
        jge		loc_PutPixel_exit

        mov		ebx, x
        cmp		ebx, clipLeft
        jl		loc_PutPixel_exit
        cmp		ebx, clipRight
        jge		loc_PutPixel_exit

        mov		ecx, pitch
        mul		ecx
        add		eax, ebx
        add		eax, ebx
        mov		edi, frameBuffer
        add		edi, eax
        mov		eax, color
        mov		[edi], ax
loc_PutPixel_exit:
    }
#else
    __asm__(
        "mov %3, %%edx;"
        "cmp %%edx, %%eax;"
        "jl loc_PutPixel_exit;"
        "mov %4, %%edx;"
        "cmp %%edx, %%eax;"
        "jge loc_PutPixel_exit;"
        "mov %5, %%edx;"
        "cmp %%edx, %%ebx;"
        "jl loc_PutPixel_exit;"
        "mov %6, %%edx;"
        "cmp %%edx, %%ebx;"
        "jge loc_PutPixel_exit;"
        "mul %%ecx;"
        "add %%ebx, %%eax;"
        "add %%ebx, %%eax;"
    #ifdef __x86_64__
        "addq %%rax, %%rdi;"
        "mov %2, %%eax;"
        "movw %%ax, (%%rdi);"
    #else
        "add %%eax, %%edi;"
        "mov %2, %%eax;"
        "movw %%ax, (%%edi);"
    #endif
        "loc_PutPixel_exit:"
        :
        :"b"(x), "a"(y), "g"(color), "g"(clipTop), "g"(clipBottom), "g"(clipLeft), "g"(clipRight), "c"(pitch), "D"(frameBuffer)
    #ifdef __x86_64__
        :"%rax", "%ebx", "%ecx", "%edx", "%rdi", "memory"
    #else
        :"%eax", "%ebx", "%ecx", "%edx", "%edi", "memory"
    #endif
    );
#endif
}

void DrawPixelFast(int x, int y, int color) {
    void*   frameBuffer = GpCanvas->frameBuffer;
    long    pitch       = GpCanvas->pitch;

#ifndef __GNUC__
    __asm {
        mov		eax, y
        mov		ebx, x
        mov		ecx, pitch
        mul		ecx
        add		eax, ebx
        add		eax, ebx
        mov		edi, frameBuffer
        add		edi, eax
        mov		eax, color
        mov		[edi], ax
    }
#else
    __asm__(
        "mul %%ecx;"
        "add %%ebx, %%eax;"
        "add %%ebx, %%eax;"
    #ifdef __x86_64__
        "addq %%rax, %%rdi;"
        "mov %3, %%eax;"
        "movw %%ax, (%%rdi);"
    #else
        "add %%eax, %%edi;"
        "mov %3, %%eax;"
        "movw %%ax, (%%edi);"
    #endif
        :
        :"b"(x), "a"(y), "c"(pitch), "g"(color), "D"(frameBuffer)
    #ifdef __x86_64__
        :"%rax", "%ebx", "%rdi", "memory"
    #else
        :"%eax", "%ebx", "%edi", "memory"
    #endif
    );
#endif
}

void DrawPixelAlpha(int x, int y, int color, int alpha) {
    void*   frameBuffer = GpCanvas->frameBuffer;
    long    pitch       = GpCanvas->pitch;

    long    clipLeft    = GpCanvas->rect.left;
    long    clipTop     = GpCanvas->rect.top;
    long    clipRight   = GpCanvas->rect.right;
    long    clipBottom  = GpCanvas->rect.bottom;

#ifndef __GNUC__
    __asm
    {
        mov     eax, y
        cmp		eax, clipTop
        jl		loc_PutPixelAlpha_exit
        cmp		eax, clipBottom
        jge		loc_PutPixelAlpha_exit

        mov		ebx, x
        cmp		ebx, clipLeft
        jl		loc_PutPixelAlpha_exit
        cmp		ebx, clipRight
        jge		loc_PutPixelAlpha_exit

        mov		ecx, pitch
        mul		ecx
        add     eax, ebx
        add     eax, ebx
        mov		edi, frameBuffer
        add     edi, eax

        mov     ecx, color
        mov		ax, cx
        shl		eax, 16
        mov		ax, cx
        and		eax, 0x07e0f81f
        mov		cx, [edi]
        mov		bx, cx
        shl		ebx, 16
        mov		bx, cx
        and		ebx, 0x07e0f81f
        mov		ecx, alpha
        mul		ecx
        neg		ecx
        add		ecx, 32
        xchg	eax, ebx
        mul		ecx
        add		eax, ebx
        shr		eax, 5
        and		eax, 0x07e0f81f
        mov		cx, ax
        shr		eax, 16
        or		ax, cx
        mov     [edi], ax
loc_PutPixelAlpha_exit:
    }
#else
    __asm__(
        "mov %4, %%edx;"
        "cmp %%edx, %%eax;"
        "jl loc_PutPixelAlpha_exit;"
        "mov %5, %%edx;"
        "cmp %%edx, %%eax;"
        "jge loc_PutPixelAlpha_exit;"
        "mov %6, %%edx;"
        "cmp %%edx, %%ebx;"
        "jl loc_PutPixelAlpha_exit;"
        "mov %7, %%edx;"
        "cmp %%edx, %%ebx;"
        "jge loc_PutPixelAlpha_exit;"
        "mul %%ecx;"
        "add %%ebx, %%eax;"
        "add %%ebx, %%eax;"
    #ifdef __x86_64__
        "addq %%rax, %%rdi;"
    #else
        "add %%eax, %%edi;"
    #endif
        "mov %2, %%ecx;"
        "movw %%cx, %%ax;"
        "shl $16, %%eax;"
        "movw %%cx, %%ax;"
        "and $132184095, %%eax;"
    #ifdef __x86_64__
        "mov (%%rdi), %%cx;"
    #else
        "mov (%%edi), %%cx;"
    #endif
        "movw %%cx, %%bx;"
        "shl $16, %%ebx;"
        "movw %%cx, %%bx;"
        "and $132184095, %%ebx;"
        "mov %3, %%ecx;"
        "mul %%ecx;"
        "neg %%ecx;"
        "add $32, %%ecx;"
        "xchg %%ebx, %%eax;"
        "mul %%ecx;"
        "add %%ebx, %%eax;"
        "shr $5, %%eax;"
        "and $132184095, %%eax;"
        "movw %%ax, %%cx;"
        "shr $16, %%eax;"
        "orw %%cx, %%ax;"
    #ifdef __x86_64__
        "movw %%ax, (%%rdi);"
    #else
        "movw %%ax, (%%edi);"
    #endif


        "loc_PutPixelAlpha_exit:"
        :
        :"b"(x), "a"(y), "g"(color), "g"(alpha), "g"(clipTop), "g"(clipBottom), "g"(clipLeft), "g"(clipRight), "c"(pitch), "D"(frameBuffer)
    #ifdef __x86_64__
        :"%rax", "%ebx", "%ecx", "%edx", "%rdi", "memory"
    #else
        :"%eax", "%ebx", "%ecx", "%edx", "%edi", "memory"
    #endif
    );
#endif
}

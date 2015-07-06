#include "Color.h"

typedef struct {
    int        Red;
    int        Green;
    int        Blue;
} Palette;

static Palette m_Alpha = {0, 0, 0};

WORD (*GpRGB)(int nRed, int nGreen, int nBlue) = RGB565;
void (*GpHigh2True)(Pal16 Pal16, Pal24* Pal24) = High2True565;

void SetAlphaValue(int Red, int Green, int Blue) {
    m_Alpha.Red     = Red;
    m_Alpha.Green   = Green;
    m_Alpha.Blue    = Blue;
}

void Pal16To24(Pal16* pPal16, Pal24* pPal24, int nColors) {
    while (nColors-- > 0) {
        GpHigh2True(*pPal16, pPal24);
        pPal16++;
        pPal24++;
    }
}

void Pal16To32(Pal16* pPal16, Pal32* pPal32, int nColors) {
    Pal24 Pal24;
    while (nColors-- > 0) {
        GpHigh2True(*pPal16, &Pal24);
        pPal32->Red     = Pal24.Red;
        pPal32->Green   = Pal24.Green;
        pPal32->Blue    = Pal24.Blue;
        pPal16++;
        pPal32++;
    }
}

void Pal24To16(Pal24* pPal24, Pal16* pPal16, int nColors) {
    while (nColors-- > 0) {
        *pPal16 = GpRGB(pPal24->Red, pPal24->Green, pPal24->Blue);
        pPal16++;
        pPal24++;
    }
}

void Pal24To32(Pal24* pPal24, Pal32* pPal32, int nColors) {
    for (int i = 0; i < nColors; i++) {
        pPal32[i].Red   = pPal24[i].Red;
        pPal32[i].Green = pPal24[i].Green;
        pPal32[i].Blue  = pPal24[i].Blue;
        pPal32[i].Alpha = 0;
    }
}

void Pal32To16(Pal32* pPal32, Pal16* pPal16, int nColors) {
    while (nColors-- > 0) {
        *pPal16 = GpRGB(pPal32->Red, pPal32->Green, pPal32->Blue);
        pPal16++;
        pPal32++;
    }
}

void Pal32To24(Pal32* pPal32, Pal24* pPal24, int nColors) {
    for (int i = 0; i < nColors; i++) {
        pPal24[i].Red   = pPal32[i].Red;
        pPal24[i].Green = pPal32[i].Green;
        pPal24[i].Blue  = pPal32[i].Blue;
    }
}

void Pal16To16Blend(Pal16* pPal1, Pal16* pPal2, int nColors) {
    int Red, Green, Blue;
    Pal24 Pal24;
    while (nColors-- > 0) {
        GpHigh2True(*pPal1, &Pal24);
        Red = Pal24.Red * m_Alpha.Red / 255;
        if (Red > 255)
            Red = 255;
        Green = Pal24.Green * m_Alpha.Green / 255;
        if (Green > 255)
            Green = 255;
        Blue = Pal24.Blue * m_Alpha.Blue / 255;
        if (Blue > 255)
            Blue = 255;
        *pPal2 = GpRGB(Red, Green, Blue);
        pPal1++;
        pPal2++;
    }
}

void Pal24To16Blend(Pal24* pPal24, Pal16* pPal16, int nColors) {
    int Red, Green, Blue;
    while (nColors-- > 0) {
        Red = pPal24->Red * m_Alpha.Red / 255;
        if (Red > 255)
            Red = 255;
        Green = pPal24->Green * m_Alpha.Green / 255;
        if (Green > 255)
            Green = 255;
        Blue = pPal24->Blue * m_Alpha.Blue / 255;
        if (Blue > 255)
            Blue = 255;
        *pPal16 = GpRGB(Red, Green, Blue);
        pPal16++;
        pPal24++;
    }
}

void Pal24To16Gray(Pal24* pPal24, Pal16* pPal16, int nColors) {
    BYTE Gray;
    while (nColors-- > 0) {
        Gray = RgbToGray(pPal24->Red, pPal24->Green, pPal24->Blue);
        *pPal16 = GpRGB(Gray, Gray, Gray);
        pPal16++;
        pPal24++;
    }
}

void Pal32To16Blend(Pal32* pPal32, Pal16* pPal16, int nColors) {
    int Red, Green, Blue;
    while (nColors-- > 0) {
        Red = pPal32->Red * m_Alpha.Red / 255;
        if (Red > 255)
            Red = 255;
        Green = pPal32->Green * m_Alpha.Green / 255;
        if (Green > 255)
            Green = 255;
        Blue = pPal32->Blue * m_Alpha.Blue / 255;
        if (Blue > 255)
            Blue = 255;
        *pPal16 = GpRGB(Red, Green, Blue);
        pPal16++;
        pPal32++;
    }
}

WORD RGB555(int nRed, int nGreen, int nBlue) {
    WORD wColor;
#ifndef __GNUC__
    __asm l{
        xor ecx, ecx
        mov ebx, 0xff
        mov eax, nRed
        and eax, ebx
        shr eax, 3
        shl eax, 10
        or  ecx, eax
        mov eax, nGreen
        and eax, ebx
        shr eax, 3
        shl eax, 5
        or  ecx, eax
        mov eax, nBlue
        and eax, ebx
        shr eax, 3
        or  ecx, eax
        mov wColor, cx
    }
#else
    __asm__(
        "xorl %%ecx, %%ecx;"
        "movl $0xff, %%eax;"
        "and %%eax, %1;"
        "shr $3, %1;"
        "shl $10, %1;"
        "or %1, %%ecx;"
        "and %%eax, %2;"
        "shr $3, %2;"
        "shl $5, %2;"
        "or %2, %%ecx;"
        "and %%eax, %3;"
        "shr $3, %3;"
        "or %3, %%ecx;"
        "movw %%cx, %0"
        :"=r"(wColor)
        :"r"(nRed), "r"(nGreen), "r"(nBlue)
        :"%eax", "%ecx"
    );
#endif
    return wColor;
}

WORD RGB565(int nRed, int nGreen, int nBlue) {
    WORD wColor;
#ifndef __GNUC__
    __asm {
        xor ecx, ecx
        mov ebx, 0xff
        mov eax, nRed
        and eax, ebx
        shr eax, 3
        shl eax, 11
        or  ecx, eax
        mov eax, nGreen
        and eax, ebx
        shr eax, 2
        shl eax, 5
        or  ecx, eax
        mov eax, nBlue
        and eax, ebx
        shr eax, 3
        or  ecx, eax
        mov wColor, cx
    }
#else
    __asm__(
        "xorl %%ecx, %%ecx;"
        "movl $0xff, %%eax;"
        "and %%eax, %1;"
        "shr $3, %1;"
        "shl $11, %1;"
        "or %1, %%ecx;"
        "and %%eax, %2;"
        "shr $2, %2;"
        "shl $5, %2;"
        "or %2, %%ecx;"
        "and %%eax, %3;"
        "shr $3, %3;"
        "or %3, %%ecx;"
        "movw %%cx, %0"
        :"=r"(wColor)
        :"r"(nRed), "r"(nGreen), "r"(nBlue)
        :"%eax", "%ecx"
    );
#endif
    return wColor;
}

void RGB555To565(int nWidth, int nHeight, void* lpBitmap) {
#ifndef __GNUC__
    __asm {
        mov esi, lpBitmap
        mov edx, nHeight
loc_555to565_loop1:
        mov ecx, nWidth
loc_555to565_loop2:
        mov ax, [esi]
        mov bx, ax
        shr ax, 5
        shl ax, 6
        and bx, 0x001f
        or  ax, bx
        mov [esi], ax
        add esi, 2
        dec ecx
        jnz loc_555to565_loop2
        dec edx
        jnz loc_555to565_loop1
    }
#else
    __asm__(
    "loc_555to565_loop1:"
        "mov %0, %%ecx;"
    "loc_555to565_loop2:"
        "movw (%2), %%ax;"
        "movw %%ax, %%bx;"
        "shr $5, %%ax;"
        "shl $6, %%ax;"
        "andw $0x001f, %%bx;"
        "orw %%bx, %%ax;"
        "mov %%ax, (%2);"
        "add $2, %2;"
        "decl %%ecx;"
        "jnz loc_555to565_loop2;"
        "decl %1;"
        "jnz loc_555to565_loop1;"
        :
        :"r"(nWidth), "r"(nHeight), "r"(lpBitmap)
        :"%ax", "%bx", "%ecx"
    );
#endif
}

void RGB565To555(int nWidth, int nHeight, void* lpBitmap) {
#ifndef __GNUC__
    __asm {
        mov esi, lpBitmap
        mov edx, nHeight
loc_565to555_loop1:
        mov ecx, nWidth
loc_565to555_loop2:
        mov ax, [esi]
        mov bx, ax
        shr ax, 6
        shl ax, 5
        and bx, 0x001f
        or  ax, bx
        mov [esi], ax
        add esi, 2
        dec ecx
        jnz loc_565to555_loop2
        dec edx
        jnz loc_565to555_loop1
    }
#else
    __asm__(
    "loc_565to555_loop1:"
        "mov %0, %%ecx;"
    "loc_565to555_loop2:"
        "movw (%2), %%ax;"
        "movw %%ax, %%bx;"
        "shr $6, %%ax;"
        "shl $5, %%ax;"
        "andw $0x001f, %%bx;"
        "orw %%bx, %%ax;"
        "mov %%ax, (%2);"
        "add $2, %2;"
        "decl %%ecx;"
        "jnz loc_565to555_loop2;"
        "decl %1;"
        "jnz loc_565to555_loop1;"
        :
        :"r"(nWidth), "r"(nHeight), "r"(lpBitmap)
        :"%ax", "%bx", "%ecx"
    );
#endif
}

void High2True555(Pal16 Pal16, Pal24* Pal24) {
    Pal24->Red      = ((Pal16 >> 10) & 0x1f) << 3;  // red
    Pal24->Green    = ((Pal16 >> 5) & 0x1f) << 3;   // green
    Pal24->Blue     = (Pal16 & 0x1f) << 3;          // blue
}

void High2True565(Pal16 Pal16, Pal24* Pal24) {
    Pal24->Red      = ((Pal16 >> 11) & 0x1f) << 3;  // red
    Pal24->Green    = ((Pal16 >> 5) & 0x3f) << 2;   // green
    Pal24->Blue     = (Pal16 & 0x1f) << 3;          // blue
}

BYTE RgbToGray(BYTE Red, BYTE Green, BYTE Blue) {
    return ((Red * 11 + Green * 59 + Blue * 30) / 100);
}

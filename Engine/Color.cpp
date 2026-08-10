#include "Color.h"

#include "AsmRoutines.h"

typedef struct {
    int        Red;
    int        Green;
    int        Blue;
} Palette;

static Palette m_Alpha = {0, 0, 0};

WORD (*GpRGB)(int red, int green, int blue) = DRGB565;
void (*GpHigh2True)(Pal16 Pal16, Pal24* Pal24) = DHigh2True565;

void DSetAlphaValue(int Red, int Green, int Blue) {
    m_Alpha.Red     = Red;
    m_Alpha.Green   = Green;
    m_Alpha.Blue    = Blue;
}

void DPal16To24(Pal16* pPal16, Pal24* pPal24, int nColors) {
    while (nColors-- > 0) {
        GpHigh2True(*pPal16, pPal24);
        pPal16++;
        pPal24++;
    }
}

void DPal16To32(Pal16* pPal16, Pal32* pPal32, int nColors) {
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

void DPal24To16(Pal24* pPal24, Pal16* pPal16, int nColors) {
    while (nColors-- > 0) {
        *pPal16 = GpRGB(pPal24->Red, pPal24->Green, pPal24->Blue);
        pPal16++;
        pPal24++;
    }
}

void DPal24To32(Pal24* pPal24, Pal32* pPal32, int nColors) {
    for (int i = 0; i < nColors; i++) {
        pPal32[i].Red   = pPal24[i].Red;
        pPal32[i].Green = pPal24[i].Green;
        pPal32[i].Blue  = pPal24[i].Blue;
        pPal32[i].Alpha = 0;
    }
}

void DPal32To16(Pal32* pPal32, Pal16* pPal16, int nColors) {
    while (nColors-- > 0) {
        *pPal16 = GpRGB(pPal32->Red, pPal32->Green, pPal32->Blue);
        pPal16++;
        pPal32++;
    }
}

void DPal32To24(Pal32* pPal32, Pal24* pPal24, int nColors) {
    for (int i = 0; i < nColors; i++) {
        pPal24[i].Red   = pPal32[i].Red;
        pPal24[i].Green = pPal32[i].Green;
        pPal24[i].Blue  = pPal32[i].Blue;
    }
}

void DPal16To16Blend(Pal16* pPal1, Pal16* pPal2, int nColors) {
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

void DPal24To16Blend(Pal24* pPal24, Pal16* pPal16, int nColors) {
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

void DPal24To16Gray(Pal24* pPal24, Pal16* pPal16, int nColors) {
    BYTE Gray;
    while (nColors-- > 0) {
        Gray = DRgbToGray(pPal24->Red, pPal24->Green, pPal24->Blue);
        *pPal16 = GpRGB(Gray, Gray, Gray);
        pPal16++;
        pPal24++;
    }
}

void DPal32To16Blend(Pal32* pPal32, Pal16* pPal16, int nColors) {
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

WORD DRGB555(int red, int green, int blue) {
    return DRGB555Core(red, green, blue);
}

WORD DRGB565(int red, int green, int blue) {
    return DRGB565Core(red, green, blue);
}

void DRGB555To565(int width, int height, void* bm_ptr) {
    DRGB555To565Core(width, height, bm_ptr);
}

void DRGB565To555(int width, int height, void* bm_ptr) {
    DRGB565To555Core(width, height, bm_ptr);
}

void DHigh2True555(Pal16 Pal16, Pal24* Pal24) {
    Pal24->Red      = ((Pal16 >> 10) & 0x1f) << 3;  // red
    Pal24->Green    = ((Pal16 >> 5) & 0x1f) << 3;   // green
    Pal24->Blue     = (Pal16 & 0x1f) << 3;          // blue
}

void DHigh2True565(Pal16 Pal16, Pal24* Pal24) {
    Pal24->Red      = ((Pal16 >> 11) & 0x1f) << 3;  // red
    Pal24->Green    = ((Pal16 >> 5) & 0x3f) << 2;   // green
    Pal24->Blue     = (Pal16 & 0x1f) << 3;          // blue
}

BYTE DRgbToGray(BYTE Red, BYTE Green, BYTE Blue) {
    return ((Red * 11 + Green * 59 + Blue * 30) / 100);
}

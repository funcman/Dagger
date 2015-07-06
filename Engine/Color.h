#ifndef COLOR_H
#define COLOR_H

#include "Base.h"

typedef struct {
    BYTE        Blue;
    BYTE        Green;
    BYTE        Red;
    BYTE        Alpha;
} Pal32;

typedef struct {
    BYTE        Red;
    BYTE        Green;
    BYTE        Blue;
} Pal24;

typedef WORD    Pal16;

void    SetAlphaValue(int Red, int Green, int Blue);

void    Pal16To24(Pal16* pPal16, Pal24* pPal24, int nColors);
void    Pal16To32(Pal16* pPal16, Pal32* pPal32, int nColors);
void    Pal16To16Blend(Pal16* pPal1, Pal16* pPal2, int nColors);

void    Pal24To16(Pal24* pPal24, Pal16* pPal16, int nColors);
void    Pal24To32(Pal24* pPal24, Pal32* pPal32, int nColors);
void    Pal24To16Blend(Pal24* pPal24, Pal16* pPal16, int nColors);
void    Pal24To16Gray(Pal24* pPal24, Pal16* pPal16, int nColors);

void    Pal32To16(Pal32* pPal32, Pal16* pPal16, int nColors);
void    Pal32To24(Pal32* pPal32, Pal24* pPal24, int nColors);
void    Pal32To16Blend(Pal32* pPal32, Pal16* pPal16, int nColors);

WORD    RGB555(int nRed, int nGreen, int nBlue);
WORD    RGB565(int nRed, int nGreen, int nBlue);
void    RGB565To555(int nWidth, int nHeight, void* lpBitmap);
void    RGB555To565(int nWidth, int nHeight, void* lpBitmap);
void    High2True555(Pal16 Pal16, Pal24* Pal24);
void    High2True565(Pal16 Pal16, Pal24* Pal24);
BYTE    RgbToGray(BYTE Red, BYTE Green, BYTE Blue);

extern  WORD (*GpRGB)(int nRed, int nGreen, int nBlue);
extern  void (*GpHigh2True)(Pal16 Pal16, Pal24* Pal24);

#endif//COLOR_H

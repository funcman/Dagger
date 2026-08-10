#ifndef DAGGER_COLOR_H
#define DAGGER_COLOR_H

#include "TypeDef.h"

typedef struct {
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
} Pal32;

typedef struct {
    BYTE Red;
    BYTE Green;
    BYTE Blue;
} Pal24;

typedef WORD Pal16;

void DSetAlphaValue(int Red, int Green, int Blue);

void DPal16To24(Pal16* pPal16, Pal24* pPal24, int nColors);
void DPal16To32(Pal16* pPal16, Pal32* pPal32, int nColors);
void DPal16To16Blend(Pal16* pPal1, Pal16* pPal2, int nColors);

void DPal24To16(Pal24* pPal24, Pal16* pPal16, int nColors);
void DPal24To32(Pal24* pPal24, Pal32* pPal32, int nColors);
void DPal24To16Blend(Pal24* pPal24, Pal16* pPal16, int nColors);
void DPal24To16Gray(Pal24* pPal24, Pal16* pPal16, int nColors);

void DPal32To16(Pal32* pPal32, Pal16* pPal16, int nColors);
void DPal32To24(Pal32* pPal32, Pal24* pPal24, int nColors);
void DPal32To16Blend(Pal32* pPal32, Pal16* pPal16, int nColors);

WORD DRGB555(int red, int green, int blue);
WORD DRGB565(int red, int green, int blue);
void DRGB565To555(int width, int height, void* bm_ptr);
void DRGB555To565(int width, int height, void* bm_ptr);
void DHigh2True555(Pal16 Pal16, Pal24* Pal24);
void DHigh2True565(Pal16 Pal16, Pal24* Pal24);
BYTE DRgbToGray(BYTE Red, BYTE Green, BYTE Blue);

extern WORD (*GpRGB)(int red, int green, int blue);
extern void (*GpHigh2True)(Pal16 Pal16, Pal24* Pal24);

#endif//DAGGER_COLOR_H

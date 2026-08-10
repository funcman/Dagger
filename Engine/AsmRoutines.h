#ifndef DAGGER_ASMROUTINES_H
#define DAGGER_ASMROUTINES_H

// Interface of the standalone assembly routines (implementations under
// asm/, one for MSVC and one for GCC/Clang). Pixel arguments are passed
// by struct pointer to hide the differences between the Windows x64 and
// System V calling conventions; the layout must match the offsets used
// by the asm/ sources.

typedef struct {
    void* frameBuffer;  // offset 0 : start of the frame buffer
    int pitch;          // offset 8 : bytes per scanline
    int x;              // offset 12
    int y;              // offset 16
    int color;          // offset 20: low 16 bits used (RGB565/RGB555)
    int alpha;          // offset 24: 0 ~ 32
} AsmPixelArgs;

extern "C" {

// Write a 16-bit pixel: *(WORD*)((BYTE*)fb + y*pitch + 2*x) = color
void AsmDrawPixel16(const AsmPixelArgs* args);
// Blend a 16-bit pixel with alpha (0~32)
void AsmDrawPixelAlpha16(const AsmPixelArgs* args);

unsigned short AsmRGB555(int red, int green, int blue);
unsigned short AsmRGB565(int red, int green, int blue);

// Convert width*height 16-bit pixels in place
void AsmRGB555To565(int width, int height, void* bm_ptr);
void AsmRGB565To555(int width, int height, void* bm_ptr);

}  // extern "C"

#endif//DAGGER_ASMROUTINES_H

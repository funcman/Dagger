#ifndef DAGGER_ASMROUTINES_H
#define DAGGER_ASMROUTINES_H

// Interface of the standalone assembly routines (implementations under
// asm/, one for MSVC and one for GCC/Clang). Arguments are passed by
// struct pointer to hide the differences between the Windows x64 and
// System V calling conventions; the layout must match the offsets used
// by the asm/ sources.

typedef struct {
    void* frameBuffer;  // offset 0 : start of the frame buffer
    int pitch;          // offset 8 : bytes per scanline
    int x;              // offset 12
    int y;              // offset 16
    int color;          // offset 20: low 16 bits used (RGB565/RGB555)
    int alpha;          // offset 24: 0 ~ 32
} DPixelArgs;

typedef struct {
    void* frameBuffer;  // offset 0 : start of the frame buffer
    int pitch;          // offset 8 : bytes per canvas scanline
    int x;              // offset 12: clipped destination x
    int y;              // offset 16: clipped destination y
    int width;          // offset 20: clipped blit width
    int height;         // offset 24: clipped blit height
    int bmpWidth;       // offset 28: full bitmap width in pixels
    int bmpHeight;      // offset 32: full bitmap height in pixels
    int srcX;           // offset 36: first source column (clipper.left)
    int srcY;           // offset 40: first source row (clipper.top)
    void* bitmap;       // offset 48: source pixels (8-bit indices / 16-bit colors)
    void* palette;      // offset 56: 8bpp -> 16-bit color table (256 entries)
} DBitmapArgs;

extern "C" {

// Write a 16-bit pixel: *(WORD*)((BYTE*)fb + y*pitch + 2*x) = color
void DDrawPixel16Core(const DPixelArgs* args);
// Blend a 16-bit pixel with alpha (0~32)
void DDrawPixelAlpha16Core(const DPixelArgs* args);

unsigned short DRGB555Core(int red, int green, int blue);
unsigned short DRGB565Core(int red, int green, int blue);

// Convert width*height 16-bit pixels in place
void DRGB555To565Core(int width, int height, void* bmPtr);
void DRGB565To555Core(int width, int height, void* bmPtr);

// Bitmap blitters. Clipping happens in the caller (see Bitmap.cpp);
// these only see the clipped rectangle through DBitmapArgs.
void DDrawBitmapCore(const DBitmapArgs* args);       // 8bpp, palette lookup
void DDrawBitmapTransCore(const DBitmapArgs* args);  // 8bpp, 50% blend with the canvas
void DDrawBitmap16Core(const DBitmapArgs* args);     // 16bpp copy
void DDrawBitmap16MmxCore(const DBitmapArgs* args);  // 16bpp copy, 8 bytes per iteration
void DDrawBitmap16RevCore(const DBitmapArgs* args);  // 16bpp copy, vertically flipped

}  // extern "C"

#endif//DAGGER_ASMROUTINES_H

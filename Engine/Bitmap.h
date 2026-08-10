#ifndef DAGGER_BITMAP_H
#define DAGGER_BITMAP_H

#include "Color.h"
#include "Memory.h"
#include "TypeDef.h"

void DDrawBmp(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette);
void DDrawBmpTrans(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette);
void DDrawBmp16(int nX, int nY, int nWidth, int nHeight, void* pBitmap);
void DDrawBmp16Mmx(int nX, int nY, int nWidth, int nHeight, void* pBitmap);
void DDrawBmp16Rev(int nX, int nY, int nWidth, int nHeight, void* pBitmap);

class DBitmap {
   public:
    DMemory bitmap;
    DMemory pal32;
    DMemory pal16;
    long width;
    long height;
    long bpp;
    long colors;

   public:
    DBitmap();
    bool New(int width, int height, int bpp);
    void Fill(BYTE fill);
    void Draw(int x, int y);
    void MakePalette();
    void ShiftAlphaBits(int bits);
    void CreateAlphaLigth(DBitmap* bm);
    void CreateAlphaTrans(DBitmap* bm);

    void* GetBitmap() {
        return bitmap.GetMemPtr();
    };

    Pal32* GetPalette() {
        return (Pal32*)pal32.GetMemPtr();
    };

    Pal16* GetPalette16() {
        return (Pal16*)pal16.GetMemPtr();
    };

    long GetBpp() {
        return bpp;
    };

    long GetWidth() {
        return width;
    };

    long GetHeight() {
        return height;
    };

    long GetColors() {
        return colors;
    };

    BYTE* GetPointer(int x, int y);
};

#endif//DAGGER_BITMAP_H

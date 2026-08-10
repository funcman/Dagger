#include "Bitmap.h"

#include "AsmRoutines.h"
#include "Canvas.h"
#include "Debug.h"

// Clip against the canvas and pack the blit arguments for the asm cores.
static bool MakeBitmapArgs(int x, int y, int width, int height, void* bmPtr, void* palPtr, DBitmapArgs* args) {
    DClipper clipper = {x, y, width, height, 0, 0, 0, 0};
    if (!GpCanvas->makeClipper(&clipper))
        return false;

    args->frameBuffer = GpCanvas->frameBuffer;
    args->pitch = (int)GpCanvas->pitch;
    args->x = (int)clipper.x;
    args->y = (int)clipper.y;
    args->width = (int)clipper.width;
    args->height = (int)clipper.height;
    args->bmpWidth = width;
    args->bmpHeight = height;
    args->srcX = (int)clipper.left;
    args->srcY = (int)clipper.top;
    args->bitmap = bmPtr;
    args->palette = palPtr;
    return true;
}

void DDrawBitmap(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette) {
    DBitmapArgs args;
    if (!MakeBitmapArgs(nX, nY, nWidth, nHeight, pBitmap, pPalette, &args))
        return;
    DDrawBitmapCore(&args);
}

void DDrawBitmapTrans(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette) {
    DBitmapArgs args;
    if (!MakeBitmapArgs(nX, nY, nWidth, nHeight, pBitmap, pPalette, &args))
        return;
    DDrawBitmapTransCore(&args);
}

void DDrawBitmap16(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    DBitmapArgs args;
    if (!MakeBitmapArgs(nX, nY, nWidth, nHeight, pBitmap, nullptr, &args))
        return;
    DDrawBitmap16Core(&args);
}

void DDrawBitmap16Mmx(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    DBitmapArgs args;
    if (!MakeBitmapArgs(nX, nY, nWidth, nHeight, pBitmap, nullptr, &args))
        return;
    DDrawBitmap16MmxCore(&args);
}

void DDrawBitmap16Rev(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    DBitmapArgs args;
    if (!MakeBitmapArgs(nX, nY, nWidth, nHeight, pBitmap, nullptr, &args))
        return;
    DDrawBitmap16RevCore(&args);
}

DBitmap::DBitmap() {
    width = 0;
    height = 0;
    bpp = 0;
    colors = 0;
}

bool DBitmap::New(int nWidth, int nHeight, int nBpp) {
    if (nBpp != 8 && nBpp != 16 && nBpp != 24 && nBpp != 32) {
        DDebugLog("DBitmap::New(%d, %d, %d) Failed.", nWidth, nHeight, nBpp);
        return false;
    }
    width = nWidth;
    height = nHeight;
    bpp = nBpp;
    bitmap.Alloc(width * height * bpp / 8);
    if (bpp == 8) {
        colors = 256;
        pal32.Alloc(256 * sizeof(Pal32));
        pal16.Alloc(256 * sizeof(Pal16));
    }
    return true;
}

void DBitmap::Fill(BYTE fill) {
    bitmap.Fill(fill);
}

void DBitmap::MakePalette() {
    if (bpp == 8)
        DPal32To16((Pal32*)pal32.GetMemPtr(), (Pal16*)pal16.GetMemPtr(), colors);
}

void DBitmap::Draw(int x, int y) {
    void* bmPtr = bitmap.GetMemPtr();
    void* palPtr = pal16.GetMemPtr();
    if (!bmPtr)
        return;
    switch (bpp) {
        case 8:
            DDrawBitmap(x, y, width, height, bmPtr, palPtr);
            break;

        case 16:
            DDrawBitmap16Mmx(x, y, width, height, bmPtr);
            break;
    }
}

void DBitmap::ShiftAlphaBits(int bits) {
    if (bpp != 8)
        return;
    BYTE* bmPtr = (BYTE*)bitmap.GetMemPtr();
    for (long i = 0; i < width * height; i++) {
        *bmPtr >>= bits;
        *bmPtr <<= bits;
        if (*bmPtr > 0)
            *bmPtr += ((1 << bits) - 1);
        bmPtr++;
    }
}

void DBitmap::CreateAlphaLigth(DBitmap* bm) {
    Fill(0);
    BYTE* image = (BYTE*)bm->GetBitmap();
    BYTE* alpha = (BYTE*)GetBitmap();
    BYTE colorKey = *image;
    int a;
    Pal32* pal = bm->GetPalette();
    for (long i = 0; i < width * height; i++) {
        if (colorKey != *image) {
            a = DRgbToGray(pal[*image].Red, pal[*image].Green, pal[*image].Blue);
            a = ((a * 2) / 16) * 16;
            if (a > 255)
                a = 255;
            *alpha = (BYTE)a;
        }
        image++;
        alpha++;
    }
}

void DBitmap::CreateAlphaTrans(DBitmap* bm) {
    Fill(0);
    BYTE* image = (BYTE*)bm->GetBitmap();
    BYTE* alpha = (BYTE*)GetBitmap();
    BYTE colorKey = *image;
    for (long i = 0; i < width * height; i++) {
        if (colorKey != *image) {
            *alpha = 255;
        }
        image++;
        alpha++;
    }
}

BYTE* DBitmap::GetPointer(int x, int y) {
    BYTE* bmPtr = (BYTE*)bitmap.GetMemPtr();
    bmPtr += (y * width + x) * bpp / 8;
    return bmPtr;
}

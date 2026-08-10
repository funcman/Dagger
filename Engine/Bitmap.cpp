#include "Bitmap.h"

#include "Canvas.h"
#include "Debug.h"

#define DBMP_MASK16 0xF7DE

void DDrawBmp(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette) {
    Clipper clipper = {nX, nY, nWidth, nHeight, 0, 0, 0, 0};
    if (!GpCanvas->makeClipper(&clipper))
        return;

    BYTE* dst = (BYTE*)GpCanvas->frameBuffer + clipper.y * GpCanvas->pitch + clipper.x * 2;
    const BYTE* src = (const BYTE*)pBitmap + clipper.top * nWidth + clipper.left;
    const Pal16* pal = (const Pal16*)pPalette;

    for (long row = 0; row < clipper.height; row++) {
        WORD* dstPixel = (WORD*)dst;
        for (long col = 0; col < clipper.width; col++)
            *dstPixel++ = pal[*src++];
        dst += GpCanvas->pitch;
        src += nWidth - clipper.width;
    }
}

void DDrawBmpTrans(int nX, int nY, int nWidth, int nHeight, void* pBitmap, void* pPalette) {
    Clipper clipper = {nX, nY, nWidth, nHeight, 0, 0, 0, 0};
    if (!GpCanvas->makeClipper(&clipper))
        return;

    BYTE* dst = (BYTE*)GpCanvas->frameBuffer + clipper.y * GpCanvas->pitch + clipper.x * 2;
    const BYTE* src = (const BYTE*)pBitmap + clipper.top * nWidth + clipper.left;
    const Pal16* pal = (const Pal16*)pPalette;

    for (long row = 0; row < clipper.height; row++) {
        WORD* dstPixel = (WORD*)dst;
        for (long col = 0; col < clipper.width; col++) {
            WORD color = pal[*src++];
            *dstPixel = (WORD)(((color & DBMP_MASK16) >> 1) + ((*dstPixel & DBMP_MASK16) >> 1));
            dstPixel++;
        }
        dst += GpCanvas->pitch;
        src += nWidth - clipper.width;
    }
}

void DDrawBmp16(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    Clipper clipper = {nX, nY, nWidth, nHeight, 0, 0, 0, 0};
    if (!GpCanvas->makeClipper(&clipper))
        return;

    WORD* dst = (WORD*)((BYTE*)GpCanvas->frameBuffer + clipper.y * GpCanvas->pitch + clipper.x * 2);
    const WORD* src = (const WORD*)pBitmap + clipper.top * nWidth + clipper.left;

    for (long row = 0; row < clipper.height; row++) {
        for (long col = 0; col < clipper.width; col++)
            *dst++ = *src++;
        dst = (WORD*)((BYTE*)dst + (GpCanvas->pitch - clipper.width * 2));
        src += nWidth - clipper.width;
    }
}

void DDrawBmp16Mmx(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    // The MMX variant was only a speed-up of DDrawBmp16; same semantics.
    DDrawBmp16(nX, nY, nWidth, nHeight, pBitmap);
}

void DDrawBmp16Rev(int nX, int nY, int nWidth, int nHeight, void* pBitmap) {
    Clipper clipper = {nX, nY, nWidth, nHeight, 0, 0, 0, 0};
    if (!GpCanvas->makeClipper(&clipper))
        return;

    WORD* dst = (WORD*)((BYTE*)GpCanvas->frameBuffer + clipper.y * GpCanvas->pitch + clipper.x * 2);
    // Vertically flipped: the first canvas row reads the bitmap's last row.
    const WORD* src = (const WORD*)pBitmap + (nHeight - 1 - clipper.top) * nWidth + clipper.left;

    for (long row = 0; row < clipper.height; row++) {
        for (long col = 0; col < clipper.width; col++)
            *dst++ = *src++;
        dst = (WORD*)((BYTE*)dst + (GpCanvas->pitch - clipper.width * 2));
        src -= nWidth + clipper.width;
    }
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
        Pal32To16((Pal32*)pal32.GetMemPtr(), (Pal16*)pal16.GetMemPtr(), colors);
}

void DBitmap::Draw(int x, int y) {
    void* bmPtr = bitmap.GetMemPtr();
    void* palPtr = pal16.GetMemPtr();
    if (!bmPtr)
        return;
    switch (bpp) {
        case 8:
            DDrawBmp(x, y, width, height, bmPtr, palPtr);
            break;

        case 16:
            DDrawBmp16Mmx(x, y, width, height, bmPtr);
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
            a = RgbToGray(pal[*image].Red, pal[*image].Green, pal[*image].Blue);
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

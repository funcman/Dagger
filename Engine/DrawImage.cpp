#include "DrawImage.h"

#include "Canvas.h"
#include "Color.h"

// RGB565 channel mask used to blend two pixels at once: .g.r.b lanes.
const DWORD DRGB565_MASK = 0x07e0f81f;

static WORD Blend565(WORD src, WORD dst, int alpha) {
    DWORD s = (((DWORD)src << 16) | src) & DRGB565_MASK;
    DWORD d = (((DWORD)dst << 16) | dst) & DRGB565_MASK;
    DWORD c = ((s * alpha + d * (32 - alpha)) >> 5) & DRGB565_MASK;
    return (WORD)((c >> 16) | c);
}

static bool Inside(int x, int y) {
    return x >= GpCanvas->rect.left && x < GpCanvas->rect.right &&
           y >= GpCanvas->rect.top && y < GpCanvas->rect.bottom;
}

void DDrawImage(int x, int y, int width, int height, void* bitmap, void* palette) {
    WORD* fb = (WORD*)GpCanvas->frameBuffer;
    long pitch = GpCanvas->pitch / 2;
    BYTE* src = (BYTE*)bitmap;
    Pal16* pal = (Pal16*)palette;
    for (int row = 0; row < height; row++) {
        int col = 0;
        while (col < width) {
            int count = *src++;
            int alpha = *src++;
            if (alpha) {
                for (int i = 0; i < count; i++) {
                    if (Inside(x + col + i, y + row))
                        fb[(y + row) * pitch + x + col + i] = pal[src[i]];
                }
                src += count;
            }
            col += count;
        }
    }
}

void DDrawImageAlpha(int x, int y, int width, int height, void* bitmap, void* palette) {
    WORD* fb = (WORD*)GpCanvas->frameBuffer;
    long pitch = GpCanvas->pitch / 2;
    BYTE* src = (BYTE*)bitmap;
    Pal16* pal = (Pal16*)palette;
    for (int row = 0; row < height; row++) {
        int col = 0;
        while (col < width) {
            int count = *src++;
            int alpha = *src++;
            if (alpha) {
                for (int i = 0; i < count; i++) {
                    int px = x + col + i;
                    int py = y + row;
                    if (Inside(px, py)) {
                        if (alpha >= 0xc0)
                            fb[py * pitch + px] = pal[src[i]];
                        else
                            fb[py * pitch + px] = Blend565(pal[src[i]], fb[py * pitch + px], alpha >> 3);
                    }
                }
                src += count;
            }
            col += count;
        }
    }
}

void DDrawImageTrans(int x, int y, int width, int height, void* bitmap, void* palette, int trans) {
    WORD* fb = (WORD*)GpCanvas->frameBuffer;
    long pitch = GpCanvas->pitch / 2;
    BYTE* src = (BYTE*)bitmap;
    Pal16* pal = (Pal16*)palette;
    for (int row = 0; row < height; row++) {
        int col = 0;
        while (col < width) {
            int count = *src++;
            int alpha = *src++;
            if (alpha) {
                int blend = (((alpha >> 3) | 7) * trans) >> 5;
                for (int i = 0; i < count; i++) {
                    int px = x + col + i;
                    int py = y + row;
                    if (Inside(px, py))
                        fb[py * pitch + px] = Blend565(pal[src[i]], fb[py * pitch + px], blend);
                }
                src += count;
            }
            col += count;
        }
    }
}

void DDrawImg16(int x, int y, int width, int height, void* bitmap) {
    WORD* fb = (WORD*)GpCanvas->frameBuffer;
    long pitch = GpCanvas->pitch / 2;
    BYTE* src = (BYTE*)bitmap;
    for (int row = 0; row < height; row++) {
        int col = 0;
        while (col < width) {
            int count = *src++;
            int alpha = *src++;
            if (alpha) {
                WORD* pixel = (WORD*)src;
                for (int i = 0; i < count; i++) {
                    if (Inside(x + col + i, y + row))
                        fb[(y + row) * pitch + x + col + i] = pixel[i];
                }
                src += count * 2;
            }
            col += count;
        }
    }
}

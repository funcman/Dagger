// ==========================================================================
// Engine64_fallback.cpp - C++ fallback for Engine pixel/color/bitmap routines.
//
// Used on architectures that do not have an assembly implementation. The
// functions below match the contract in Engine/AsmRoutines.h and produce
// the same results as the x86-64 assembly versions for the common paths.
// ==========================================================================
#include "AsmRoutines.h"

#include <cstdint>

extern "C" {

void DDrawPixel16Core(const DPixelArgs* args) {
    auto* fb = static_cast<uint8_t*>(args->frameBuffer);
    auto* pixel = reinterpret_cast<uint16_t*>(fb + args->y * args->pitch + 2 * args->x);
    *pixel = static_cast<uint16_t>(args->color);
}

void DDrawPixelAlpha16Core(const DPixelArgs* args) {
    auto* fb = static_cast<uint8_t*>(args->frameBuffer);
    auto* pixel = reinterpret_cast<uint16_t*>(fb + args->y * args->pitch + 2 * args->x);

    const uint16_t dstColor = *pixel;
    const uint32_t alpha = static_cast<uint32_t>(args->alpha);

    // SIMD-within-a-register expansion used by the x86-64 assembly.
    const uint32_t src = ((static_cast<uint32_t>(args->color) << 16) | args->color) & 0x07e0f81f;
    const uint32_t dst = ((static_cast<uint32_t>(dstColor) << 16) | dstColor) & 0x07e0f81f;

    uint32_t blended = (src * alpha + dst * (32 - alpha)) >> 5;
    blended &= 0x07e0f81f;
    *pixel = static_cast<uint16_t>((blended >> 16) | (blended & 0xffff));
}

unsigned short DRGB555Core(int red, int green, int blue) {
    return static_cast<uint16_t>(
        ((red & 0xff) >> 3) << 10 |
        ((green & 0xff) >> 3) << 5 |
        ((blue & 0xff) >> 3));
}

unsigned short DRGB565Core(int red, int green, int blue) {
    return static_cast<uint16_t>(
        ((red & 0xff) >> 3) << 11 |
        ((green & 0xff) >> 2) << 5 |
        ((blue & 0xff) >> 3));
}

void DRGB555To565Core(int width, int height, void* bmPtr) {
    auto* p = static_cast<uint16_t*>(bmPtr);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint16_t c = *p;
            // Shift red and green one bit to the left; keep blue.
            *p = static_cast<uint16_t>(((c & 0x7fe0) << 1) | (c & 0x001f));
            ++p;
        }
    }
}

void DRGB565To555Core(int width, int height, void* bmPtr) {
    auto* p = static_cast<uint16_t*>(bmPtr);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint16_t c = *p;
            // Shift red and green one bit to the right; keep blue.
            *p = static_cast<uint16_t>(((c & 0xffc0) >> 1) | (c & 0x001f));
            ++p;
        }
    }
}

void DDrawBitmapCore(const DBitmapArgs* args) {
    auto* dst = static_cast<uint8_t*>(args->frameBuffer) + args->y * args->pitch + args->x * 2;
    auto* src = static_cast<const uint8_t*>(args->bitmap) + args->srcY * args->bmpWidth + args->srcX;
    auto* pal = static_cast<const uint16_t*>(args->palette);

    for (int row = 0; row < args->height; ++row) {
        auto* dstPixel = reinterpret_cast<uint16_t*>(dst);
        for (int col = 0; col < args->width; ++col)
            *dstPixel++ = pal[*src++];
        dst += args->pitch;
        src += args->bmpWidth - args->width;
    }
}

void DDrawBitmapTransCore(const DBitmapArgs* args) {
    auto* dst = static_cast<uint8_t*>(args->frameBuffer) + args->y * args->pitch + args->x * 2;
    auto* src = static_cast<const uint8_t*>(args->bitmap) + args->srcY * args->bmpWidth + args->srcX;
    auto* pal = static_cast<const uint16_t*>(args->palette);

    for (int row = 0; row < args->height; ++row) {
        auto* dstPixel = reinterpret_cast<uint16_t*>(dst);
        for (int col = 0; col < args->width; ++col) {
            // 50% blend via the 0x07e0f81f dual-word expansion,
            // same as the x86-64 assembly.
            const uint16_t srcColor = pal[*src++];
            const uint32_t s = ((static_cast<uint32_t>(srcColor) << 16) | srcColor) & 0x07e0f81f;
            const uint32_t d = ((static_cast<uint32_t>(*dstPixel) << 16) | *dstPixel) & 0x07e0f81f;
            const uint32_t blended = ((s + d) >> 1) & 0x07e0f81f;
            *dstPixel++ = static_cast<uint16_t>((blended >> 16) | (blended & 0xffff));
        }
        dst += args->pitch;
        src += args->bmpWidth - args->width;
    }
}

void DDrawBitmap16Core(const DBitmapArgs* args) {
    auto* dst = reinterpret_cast<uint16_t*>(
        static_cast<uint8_t*>(args->frameBuffer) + args->y * args->pitch + args->x * 2);
    auto* src = static_cast<const uint16_t*>(args->bitmap) + args->srcY * args->bmpWidth + args->srcX;

    for (int row = 0; row < args->height; ++row) {
        for (int col = 0; col < args->width; ++col)
            *dst++ = *src++;
        dst = reinterpret_cast<uint16_t*>(
            reinterpret_cast<uint8_t*>(dst) + (args->pitch - args->width * 2));
        src += args->bmpWidth - args->width;
    }
}

void DDrawBitmap16MmxCore(const DBitmapArgs* args) {
    // The MMX variant was only a speed-up of DDrawBitmap16; same semantics.
    DDrawBitmap16Core(args);
}

void DDrawBitmap16RevCore(const DBitmapArgs* args) {
    auto* dst = reinterpret_cast<uint16_t*>(
        static_cast<uint8_t*>(args->frameBuffer) + args->y * args->pitch + args->x * 2);
    // Vertically flipped: the first canvas row reads the bitmap's last row.
    auto* src = static_cast<const uint16_t*>(args->bitmap) +
                (args->bmpHeight - 1 - args->srcY) * args->bmpWidth + args->srcX;

    for (int row = 0; row < args->height; ++row) {
        for (int col = 0; col < args->width; ++col)
            *dst++ = *src++;
        dst = reinterpret_cast<uint16_t*>(
            reinterpret_cast<uint8_t*>(dst) + (args->pitch - args->width * 2));
        src -= args->bmpWidth + args->width;
    }
}

}

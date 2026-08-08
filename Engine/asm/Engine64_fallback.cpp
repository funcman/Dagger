// ==========================================================================
// Engine64_fallback.cpp - C++ fallback for Engine pixel/color routines.
//
// Used on architectures that do not have an assembly implementation. The
// functions below match the contract in Engine/AsmRoutines.h and produce
// the same results as the x86-64 assembly versions for the common paths.
// ==========================================================================
#include "AsmRoutines.h"

#include <cstdint>

extern "C" {

void AsmDrawPixel16(const AsmPixelArgs* args) {
    auto* fb = static_cast<uint8_t*>(args->frameBuffer);
    auto* pixel = reinterpret_cast<uint16_t*>(fb + args->y * args->pitch + 2 * args->x);
    *pixel = static_cast<uint16_t>(args->color);
}

void AsmDrawPixelAlpha16(const AsmPixelArgs* args) {
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

unsigned short AsmRGB555(int nRed, int nGreen, int nBlue) {
    return static_cast<uint16_t>(
        ((nRed   & 0xff) >> 3) << 10 |
        ((nGreen & 0xff) >> 3) << 5  |
        ((nBlue  & 0xff) >> 3));
}

unsigned short AsmRGB565(int nRed, int nGreen, int nBlue) {
    return static_cast<uint16_t>(
        ((nRed   & 0xff) >> 3) << 11 |
        ((nGreen & 0xff) >> 2) << 5  |
        ((nBlue  & 0xff) >> 3));
}

void AsmRGB555To565(int nWidth, int nHeight, void* lpBitmap) {
    auto* p = static_cast<uint16_t*>(lpBitmap);
    for (int y = 0; y < nHeight; ++y) {
        for (int x = 0; x < nWidth; ++x) {
            const uint16_t c = *p;
            // Shift red and green one bit to the left; keep blue.
            *p = static_cast<uint16_t>(((c & 0x7fe0) << 1) | (c & 0x001f));
            ++p;
        }
    }
}

void AsmRGB565To555(int nWidth, int nHeight, void* lpBitmap) {
    auto* p = static_cast<uint16_t*>(lpBitmap);
    for (int y = 0; y < nHeight; ++y) {
        for (int x = 0; x < nWidth; ++x) {
            const uint16_t c = *p;
            // Shift red and green one bit to the right; keep blue.
            *p = static_cast<uint16_t>(((c & 0xffc0) >> 1) | (c & 0x001f));
            ++p;
        }
    }
}

}

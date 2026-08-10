#include "DrawPixel.h"

#include "AsmRoutines.h"
#include "Canvas.h"
#include "TypeDef.h"

// Pixel write/blend assembly was moved to standalone files under asm/
// (one for MSVC, one for GCC/Clang); only clipping checks and argument
// forwarding remain here.

void DDrawPixel(int x, int y, int color) {
    if (y < GpCanvas->rect.top || y >= GpCanvas->rect.bottom ||
        x < GpCanvas->rect.left || x >= GpCanvas->rect.right) {
        return;
    }

    DPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, 0};
    DDrawPixel16Core(&args);
}

void DDrawPixelFast(int x, int y, int color) {
    DPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, 0};
    DDrawPixel16Core(&args);
}

void DDrawPixelAlpha(int x, int y, int color, int alpha) {
    if (y < GpCanvas->rect.top || y >= GpCanvas->rect.bottom ||
        x < GpCanvas->rect.left || x >= GpCanvas->rect.right) {
        return;
    }

    DPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, alpha};
    DDrawPixelAlpha16Core(&args);
}

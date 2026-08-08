#include "DrawPixel.h"

#include "AsmRoutines.h"
#include "Canvas.h"
#include "TypeDef.h"

// Pixel write/blend assembly was moved to standalone files under asm/
// (one for MSVC, one for GCC/Clang); only clipping checks and argument
// forwarding remain here.

void DrawPixel(int x, int y, int color) {
    if (y < GpCanvas->rect.top || y >= GpCanvas->rect.bottom ||
        x < GpCanvas->rect.left || x >= GpCanvas->rect.right) {
        return;
    }

    AsmPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, 0};
    AsmDrawPixel16(&args);
}

void DrawPixelFast(int x, int y, int color) {
    AsmPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, 0};
    AsmDrawPixel16(&args);
}

void DrawPixelAlpha(int x, int y, int color, int alpha) {
    if (y < GpCanvas->rect.top || y >= GpCanvas->rect.bottom ||
        x < GpCanvas->rect.left || x >= GpCanvas->rect.right) {
        return;
    }

    AsmPixelArgs args = {GpCanvas->frameBuffer, (int)GpCanvas->pitch, x, y, color, alpha};
    AsmDrawPixelAlpha16(&args);
}

#include "Canvas.h"
#include "Color.h"

Canvas* GpCanvas = 0;

Canvas::Canvas(int w, int h, void* scr) {
    GpCanvas = this;

    width       = w;
    height      = h;
    pitch       = w * 2;
    frameBuffer = scr;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = width - 1;
    rect.bottom = height - 1;

    GpRGB = RGB565;
    GpHigh2True = High2True565;
}

Canvas::~Canvas() {
    GpCanvas = 0;
}

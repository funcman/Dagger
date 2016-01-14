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

bool Canvas::makeClipper(Clipper* clipperPtr) {
    int xx = clipperPtr->x;
    int yy = clipperPtr->y;
    clipperPtr->top     = 0;
    clipperPtr->left    = 0;
    clipperPtr->right   = 0;

    if (clipperPtr->y < rect.top) {
        clipperPtr->y       =   rect.top;
        clipperPtr->top     =   rect.top - yy;
        clipperPtr->height  -=  clipperPtr->top;
    }
    if (clipperPtr->height <= 0) {
        return false;
    }

    if (clipperPtr->height > rect.bottom - clipperPtr->y) {
        clipperPtr->height = rect.bottom - clipperPtr->y;
    }
    if (clipperPtr->height <= 0) {
        return false;
    }

    if (clipperPtr->x < rect.left) {
        clipperPtr->x       =   rect.left;
        clipperPtr->left    =   rect.left - xx;
        clipperPtr->width   -=  clipperPtr->left;
    }
    if (clipperPtr->width <= 0) {
        return false;
    }

    if (clipperPtr->width > rect.right - clipperPtr->x) {
        clipperPtr->right   =   clipperPtr->width + clipperPtr->x - rect.right;
        clipperPtr->width   -=  clipperPtr->right;
    }
    if (clipperPtr->width <= 0) {
        return false;
    }

    return true;
}

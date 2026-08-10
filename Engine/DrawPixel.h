#ifndef DAGGER_DRAWPIXEL_H
#define DAGGER_DRAWPIXEL_H

// nColor: RGB(r, g, b)
// nAlpha: 0 ~ 32

void DDrawPixel(int x, int y, int color);
void DDrawPixelFast(int x, int y, int color);
void DDrawPixelAlpha(int x, int y, int color, int alpha);

#endif//DAGGER_DRAWPIXEL_H

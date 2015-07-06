#ifndef DRAWPIXEL_H
#define DRAWPIXEL_H

// nColor: RGB(r, g, b)
// nAlpha: 0 ~ 32

void DrawPixel(int x, int y, int color);
void DrawPixelFast(int x, int y, int color);
void DrawPixelAlpha(int x, int y, int color, int alpha);

#endif//DRAWPIXEL_H

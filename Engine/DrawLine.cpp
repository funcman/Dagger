#include "DrawLine.h"
#include "Macros.h"
#include "DrawPixel.h"

void DDrawLine(int x1, int y1, int x2, int y2, int color) {
    long d, x, y, ax, ay, sx, sy, dx, dy;

    dx = x2 - x1;
    ax = ABS(dx) << 1;
    sx = SIGN(dx);

    dy = y2 - y1;
    ay = ABS(dy) << 1;
    sy = SIGN(dy);

    x  = x1;
    y  = y1;

    if (ax > ay) {
        d = ay - (ax >> 1);
        while (x != x2) {
            DDrawPixel(x, y, color);
            if (d >= 0) {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }else {
        d = ax - (ay >> 1);
        while (y != y2) {
            DDrawPixel(x, y, color);
            if (d >= 0) {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
    DDrawPixel(x, y, color);
}

void DDrawLineAlpha(int x1, int y1, int x2, int y2, int color, int alpha) {
    long d, x, y, ax, ay, sx, sy, dx, dy;

    dx = x2 - x1;
    ax = ABS(dx) << 1;
    sx = SIGN(dx);

    dy = y2 - y1;
    ay = ABS(dy) << 1;
    sy = SIGN(dy);

    x  = x1;
    y  = y1;

    if (ax > ay) {
        d = ay - (ax >> 1);
        while (x != x2) {
            DDrawPixelAlpha(x, y, color, alpha);
            if (d >= 0) {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }else {
        d = ax - (ay >> 1);
        while (y != y2) {
            DDrawPixelAlpha(x, y, color, alpha);
            if (d >= 0) {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
    DDrawPixelAlpha(x, y, color, alpha);
}

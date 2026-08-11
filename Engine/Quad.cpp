#include "Quad.h"
#include "DrawLine.h"

#include <cstring>

DQuad::DQuad() {
    Destroy();
}

void DQuad::SetVertex(int index, int x, int y) {
    for (int i = index; i <= 3; i++) {
        vertex[i].x = x;
        vertex[i].y = y;
        ok = true;
    }
}

void DQuad::Make(int x1, int y1, int x2, int y2) {
    vertex[0].x = x1;
    vertex[0].y = y1;
    vertex[2].x = x2;
    vertex[2].y = y2;

    int h = vertex[2].y - vertex[0].y;
    int w = vertex[2].x - vertex[0].x;
    int a = (2 * h - w) / 2;
    int b = (2 * h - w) / 4;

    vertex[1].x = vertex[0].x - a;
    vertex[1].y = vertex[0].y + b;
    vertex[3].x = vertex[0].x + a + w;
    vertex[3].y = vertex[0].y + h - b;

    Sort();
    ok = true;
}

// Keeps the order 0=Top, 1=Left, 2=Bottom, 3=Right.
void DQuad::Sort() {
    int i;
    DPoint top = vertex[0];
    for (i = 1; i < 4; i++) {
        if (top.y > vertex[i].y)
            top = vertex[i];
    }
    DPoint bottom = vertex[0];
    for (i = 1; i < 4; i++) {
        if (bottom.y < vertex[i].y)
            bottom = vertex[i];
    }
    DPoint left = vertex[0];
    for (i = 1; i < 4; i++) {
        if (left.x > vertex[i].x)
            left = vertex[i];
    }
    DPoint right = vertex[0];
    for (i = 1; i < 4; i++) {
        if (right.x < vertex[i].x)
            right = vertex[i];
    }
    vertex[0] = top;
    vertex[1] = left;
    vertex[2] = bottom;
    vertex[3] = right;
}

void DQuad::Draw(int x, int y, int color) {
    if (!ok)
        return;
    DDrawLine(vertex[0].x - x, vertex[0].y - y,
        vertex[1].x - x, vertex[1].y - y, color);
    DDrawLine(vertex[0].x - x, vertex[0].y - y,
        vertex[3].x - x, vertex[3].y - y, color);
    DDrawLine(vertex[2].x - x, vertex[2].y - y,
        vertex[1].x - x, vertex[1].y - y, color);
    DDrawLine(vertex[2].x - x, vertex[2].y - y,
        vertex[3].x - x, vertex[3].y - y, color);
}

// Returns 0=point in front, 1=point inside, 2=point behind.
long DQuad::Compare(int x, int y) {
    DPoint pt[4];

    if (!ok)
        return 2;
    if (y < vertex[0].y)
        return 2;
    if (y > vertex[2].y)
        return 0;
    if (x < vertex[1].x) {
        if (y < vertex[1].y)
            return 2;
        return 0;
    }
    if (x > vertex[3].x) {
        if (y < vertex[3].y)
            return 2;
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        pt[i].x = vertex[i].x - x;
        pt[i].y = vertex[i].y - y;
    }
    if (pt[0].x * pt[1].y > pt[1].x * pt[0].y)
        return 2;
    if (pt[1].x * pt[2].y > pt[2].x * pt[1].y)
        return 0;
    if (pt[2].x * pt[3].y > pt[3].x * pt[2].y)
        return 0;
    if (pt[3].x * pt[0].y > pt[0].x * pt[3].y)
        return 2;
    return 1;
}

// Returns 0=quad in front, 1=quad inside or crossing, 2=quad behind.
long DQuad::Compare(DQuad* quad) {
    int x01 = quad->vertex[0].y + (quad->vertex[0].x >> 1);
    int x02 = vertex[0].y + (vertex[0].x >> 1);

    int y11 = quad->vertex[1].y - (quad->vertex[1].x >> 1);
    int x22 = vertex[2].y + (vertex[2].x >> 1);

    int y31 = quad->vertex[3].y - (quad->vertex[3].x >> 1);
    int y32 = vertex[3].y - (vertex[3].x >> 1);

    if (y11 < y32 && x01 < x22)
        return 2;
    else if (y31 > y32 && x01 > x02)
        return 0;

    int y12 = vertex[1].y - (vertex[1].x >> 1);
    int x21 = quad->vertex[2].y + (quad->vertex[2].x >> 1);

    if (x21 < x02 && y31 < y12)
        return 2;
    else if (x01 > x02 && y31 > y32)
        return 0;
    return 1;
}

void DQuad::GetCenter(DPoint* center) {
    center->x = (vertex[1].x + vertex[3].x) / 2;
    center->y = (vertex[0].y + vertex[2].y) / 2;
}

void DQuad::GetRect(DRect* rect) {
    rect->top    = vertex[0].y;
    rect->bottom = vertex[2].y;
    rect->left   = vertex[1].x;
    rect->right  = vertex[3].x;
}

void DQuad::Destroy() {
    std::memset(vertex, 0, sizeof(vertex));
    ok = false;
}

#ifndef DAGGER_QUAD_H
#define DAGGER_QUAD_H

#include "Canvas.h"

typedef struct {
    long x;
    long y;
} DPoint;

class DQuad {
public:
    DPoint vertex[4];
    bool ok;

public:
    DQuad();
    void SetVertex(int index, int x, int y);
    void Make(int x1, int y1, int x2, int y2);
    void Draw(int x, int y, int color);
    void Sort();
    long Compare(int x, int y);
    long Compare(DQuad* quad);
    void GetCenter(DPoint* center);
    void GetRect(DRect* rect);
    void Destroy();

    bool IsOk() {
        return ok;
    };
};

#endif//DAGGER_QUAD_H

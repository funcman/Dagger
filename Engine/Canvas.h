#ifndef DAGGER_CANVAS_H
#define DAGGER_CANVAS_H

typedef struct {
    long left;
    long top;
    long right;
    long bottom;
} DRect;

typedef struct {
    long x;
    long y;
    long width;
    long height;
    long left;
    long top;
    long right;
    long bottom;
} DClipper;

class DCanvas {
public:
    void* frameBuffer;
    long width;
    long height;
    long pitch;
    DRect rect;

public:
    DCanvas(int w, int h, void* scr);
    ~DCanvas();

    bool makeClipper(DClipper* clipperPtr);
};

extern DCanvas* GpCanvas;

#endif//DAGGER_CANVAS_H

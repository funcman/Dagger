#ifndef DAGGER_CANVAS_H
#define DAGGER_CANVAS_H

typedef struct {
    long left;
    long top;
    long right;
    long bottom;
} Rect;

typedef struct {
    long x;
    long y;
    long width;
    long height;
    long left;
    long top;
    long right;
    long bottom;
} Clipper;

class DCanvas {
   public:
    void* frameBuffer;
    long width;
    long height;
    long pitch;
    Rect rect;

   public:
    DCanvas(int w, int h, void* scr);
    ~DCanvas();

    bool makeClipper(Clipper* clipperPtr);
};

extern DCanvas* GpCanvas;

#endif//DAGGER_CANVAS_H

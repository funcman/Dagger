#ifndef CANVAS_H
#define CANVAS_H

typedef struct {
    long    left;
    long    top;
    long    right;
    long    bottom;
} Rect;

typedef struct {
    long    x;
    long    y;
    long    width;
    long    height;
    long    left;
    long    top;
    long    right;
    long    bottom;
} Clipper;

class Canvas {
public:
    void*       frameBuffer;
    long        width;
    long        height;
    long        pitch;
    Rect        rect;

public:
    Canvas(int w, int h, void* scr);
    ~Canvas();

    bool    makeClipper(Clipper* clipperPtr);
};

extern Canvas* GpCanvas;

#endif//CANVAS_H

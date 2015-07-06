#ifndef CANVAS_H
#define CANVAS_H

typedef struct {
    long    left;
    long    top;
    long    right;
    long    bottom;
} Rect;

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

//    void    resize(int width, int height);
//    bool    isPointInCanvas(int x, int y);
//    bool    isRectInCanvas(int x, int y, int width, int height);
//    void    saveToFile(char const* filename);
};

extern Canvas* GpCanvas;

#endif//CANVAS_H

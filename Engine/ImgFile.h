#ifndef DAGGER_IMGFILE_H
#define DAGGER_IMGFILE_H

#include "Bitmap.h"
#include "Color.h"
#include "Memory.h"
#include "Quad.h"
#include "TypeDef.h"

typedef struct {
    BYTE id[4];    // "SPR\0" or "IMG\0"
    WORD width;
    WORD height;
    WORD centerX;
    WORD centerY;
    WORD frames;
    WORD colors;
    WORD vertex[8];
} DImgHead;

typedef struct {
    DWORD offset;
    DWORD length;
} DImgOffs;

typedef struct {
    WORD width;
    WORD height;
    WORD offsetX;
    WORD offsetY;
    BYTE sprite[1];
} DImgFrame;

class DImgFile {
public:
    int size;
    int width;
    int height;
    int centerX;
    int centerY;
    int frames;
    int colors;
    DQuad quad;

public:
    DImgFile();
    bool Load(char const* fileName);
    void Free();
    void MakePalette();
    void Draw(int x, int y, int frame);
    void DrawImage(int x, int y, int frame, int blend);
    void DrawAlpha(int x, int y, int frame, int blend);
    void DrawTrans(int x, int y, int frame, int trans);
    void DrawBitmap(int frame, DBitmap* bitmap);
    DImgFrame* GetFrame(int frame);
    BYTE GetAlpha(int x, int y, int frame);

    int GetWidth() {
        return width;
    };
    int GetHeight() {
        return height;
    };
    int GetCenterX() {
        return centerX;
    };
    int GetCenterY() {
        return centerY;
    };
    int GetFrames() {
        return frames;
    };

private:
    DMemory buffer;
    DMemory palette;
    Pal24* pal24;
    Pal16* pal16;
    DImgOffs* offset;
    BYTE* image;
};

#endif//DAGGER_IMGFILE_H

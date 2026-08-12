#ifndef DAGGER_DRAWIMAGE_H
#define DAGGER_DRAWIMAGE_H

// RLE sprite blitters. Sprite data is rows of runs: {count, alpha, payload}.
// alpha == 0 skips count pixels, otherwise count palette indices (or 16-bit
// pixels for DDrawImg16) follow.

void DDrawImage(int x, int y, int width, int height, void* bitmap, void* palette);
void DDrawImageAlpha(int x, int y, int width, int height, void* bitmap, void* palette);
void DDrawImageTrans(int x, int y, int width, int height, void* bitmap, void* palette, int trans);
void DDrawImg16(int x, int y, int width, int height, void* bitmap);

#endif//DAGGER_DRAWIMAGE_H

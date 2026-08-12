#include "ImgFile.h"

#include <string.h>

#include "BinFile.h"
#include "DrawImage.h"

DImgFile::DImgFile() {
    pal24 = nullptr;
    pal16 = nullptr;
    offset = nullptr;
    image = nullptr;
    size = 0;
    width = 0;
    height = 0;
    centerX = 0;
    centerY = 0;
    frames = 0;
    colors = 0;
}

bool DImgFile::Load(char const* fileName) {
    static const int def[4][2] = {0, -5, -10, 0, 0, 5, 10, 0};

    Free();
    DBinFile file;
    if (!file.Open(fileName))
        return false;
    size = (int)file.Size();
    if (size == (int)DFILE_SEEK_ERROR)
        return false;
    if (!buffer.Alloc(size))
        return false;
    BYTE* p = (BYTE*)buffer.GetMemPtr();
    file.Read(p, size);

    DImgHead* header = (DImgHead*)p;
    if (memcmp(header->id, "SPR\0", 4) != 0 && memcmp(header->id, "IMG\0", 4) != 0)
        return false;
    width = header->width;
    height = header->height;
    centerX = header->centerX;
    centerY = header->centerY;
    frames = header->frames;
    colors = header->colors;

    for (int i = 0; i < 4; i++) {
        quad.vertex[i].x = header->vertex[i * 2];
        quad.vertex[i].y = header->vertex[i * 2 + 1];
        if (quad.vertex[i].x <= 0 && quad.vertex[i].y <= 0) {
            quad.vertex[i].x = centerX + def[i][0];
            quad.vertex[i].y = centerY + def[i][1];
        }
    }
    quad.ok = header->vertex[0] != 0;

    p += sizeof(DImgHead);
    if (colors > 0) {
        pal24 = (Pal24*)p;
        p += colors * sizeof(Pal24);
        MakePalette();
    }
    offset = (DImgOffs*)p;
    p += frames * sizeof(DImgOffs);
    image = p;
    return true;
}

void DImgFile::Free() {
    buffer.Free();
    palette.Free();
    pal24 = nullptr;
    pal16 = nullptr;
    offset = nullptr;
    image = nullptr;
    size = 0;
    frames = 0;
    colors = 0;
}

// Palette layout: original, bright, gray, red, green, blue tinted tables.
void DImgFile::MakePalette() {
    palette.Alloc(colors * sizeof(Pal16) * 6);
    pal16 = (Pal16*)palette.GetMemPtr();
    DPal24To16(pal24, pal16, colors);
    DSetAlphaValue(360, 360, 360);
    DPal24To16Blend(pal24, pal16 + colors, colors);
    DPal24To16Gray(pal24, pal16 + colors * 2, colors);
    DSetAlphaValue(256, 128, 128);
    DPal24To16Blend(pal24, pal16 + colors * 3, colors);
    DSetAlphaValue(128, 256, 128);
    DPal24To16Blend(pal24, pal16 + colors * 4, colors);
    DSetAlphaValue(128, 128, 256);
    DPal24To16Blend(pal24, pal16 + colors * 5, colors);
}

void DImgFile::Draw(int x, int y, int frame) {
    if (frame < 0 || frame >= frames)
        return;
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return;
    x += p->offsetX;
    y += p->offsetY;
    if (colors == 0)
        DDrawImg16(x, y, p->width, p->height, p->sprite);
    else
        DDrawImageAlpha(x, y, p->width, p->height, p->sprite, pal16);
}

void DImgFile::DrawImage(int x, int y, int frame, int blend) {
    if (frame < 0 || frame >= frames || blend > 5)
        return;
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return;
    x = x - centerX + p->offsetX;
    y = y - centerY + p->offsetY;
    if (colors == 0)
        DDrawImg16(x, y, p->width, p->height, p->sprite);
    else
        DDrawImage(x, y, p->width, p->height, p->sprite, pal16 + colors * blend);
}

void DImgFile::DrawAlpha(int x, int y, int frame, int blend) {
    if (frame < 0 || frame >= frames || blend > 5)
        return;
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return;
    x = x - centerX + p->offsetX;
    y = y - centerY + p->offsetY;
    if (colors == 0)
        DDrawImg16(x, y, p->width, p->height, p->sprite);
    else
        DDrawImageAlpha(x, y, p->width, p->height, p->sprite, pal16 + colors * blend);
}

void DImgFile::DrawTrans(int x, int y, int frame, int trans) {
    if (frame < 0 || frame >= frames)
        return;
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return;
    x = x - centerX + p->offsetX;
    y = y - centerY + p->offsetY;
    if (colors == 0)
        DDrawImg16(x, y, p->width, p->height, p->sprite);
    else
        DDrawImageTrans(x, y, p->width, p->height, p->sprite, pal16, trans);
}

void DImgFile::DrawBitmap(int frame, DBitmap* bitmap) {
    if (frame < 0 || frame >= frames)
        return;
    bitmap->New(width, height, 8);
    DPal24To32(pal24, bitmap->GetPalette(), colors);
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return;
    BYTE* img = p->sprite;
    int x = p->offsetX;
    int y = p->offsetY;
    int w = p->width;
    int h = p->height;
    while (h > 0) {
        w = p->width;
        x = p->offsetX;
        while (w > 0) {
            int count = *img++;
            int alpha = *img++;
            if (alpha) {
                memcpy(bitmap->GetPointer(x, y), img, count);
                img += count;
            }
            x += count;
            w -= count;
        }
        y++;
        h--;
    }
}

// x, y are coordinates inside the image.
BYTE DImgFile::GetAlpha(int x, int y, int frame) {
    if (frame < 0 || frame >= frames)
        return 0;
    DImgFrame* p = (DImgFrame*)(image + offset[frame].offset);
    if (p->width > width || p->height > height)
        return 0;
    x -= p->offsetX;
    y -= p->offsetY;
    if (x < 0 || x >= p->width)
        return 0;
    if (y < 0 || y >= p->height)
        return 0;
    BYTE* sprite = p->sprite;
    int w;
    int count;
    int alpha = 0;
    while (y-- > 0) {
        w = p->width;
        while (w > 0) {
            count = *sprite++;
            alpha = *sprite++;
            if (alpha)
                sprite += count;
            w -= count;
        }
    }
    // Consume x + 1 pixels so the run covering column x is the last one read.
    w = x + 1;
    while (w > 0) {
        count = *sprite++;
        alpha = *sprite++;
        if (alpha)
            sprite += count;
        w -= count;
    }
    return (BYTE)alpha;
}

DImgFrame* DImgFile::GetFrame(int frame) {
    if (frame < 0 || frame >= frames)
        return nullptr;
    return (DImgFrame*)(image + offset[frame].offset);
}

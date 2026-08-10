#include <memory>

#include "Base.h"
#include "Engine.h"
#include "Screen.h"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::unique_ptr<DMemPool> pool(new DMemPool());

    Screen screen(640, 480, "Dagger Engine");
    if (!screen.ok()) {
        return 1;
    }

    // The engine draws directly into the frontend's 16-bit frame buffer
    DCanvas canvas(screen.width(), screen.height(), screen.frameBuffer());

    while (screen.pollEvents()) {
        for (int l = 0; l < 32; ++l) {
            for (int i = 0; i < 100; ++i) {
                DDrawPixelFast(i, l, DRGB565(255, 0, 255));
            }
            for (int i = 0; i < 100; ++i) {
                DDrawPixelAlpha(i, l, DRGB565(0, 255, 0), l);
            }
        }

        DDrawLine(0, 0, 300, 300, DRGB565(0, 255, 0));
        DDrawLineAlpha(200, 200, 400, 400, DRGB565(255, 0, 0), 16);

        screen.present();
    }

    return 0;
}

#ifndef DAGGER_SCREEN_H
#define DAGGER_SCREEN_H

#include <stdint.h>

#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

// SDL3 frontend: window + renderer + RGB565 streaming texture.
// The engine core works in 16-bit color and writes directly into the
// 16-bit frame buffer exposed by frameBuffer(); present() uploads it as
// an RGB565 texture and SDL performs the 16->32 conversion while
// rendering (same approach as the Qt era: QImage::Format_RGB16 +
// drawImage).
class Screen {
   public:
    Screen(int width, int height);
    ~Screen();

    bool ok() const;

    // Pump window events; returns false when the user asked to quit
    bool pollEvents();
    // Upload the frame buffer and present one frame
    void present();

    void* frameBuffer();
    int width() const { return width_; }
    int height() const { return height_; }

   private:
    int width_;
    int height_;

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;

    std::vector<uint16_t> pixels_;  // 16-bit frame buffer, RGB565

    uint64_t fpsStart_ = 0;
    unsigned fpsFrames_ = 0;
};

#endif//DAGGER_SCREEN_H

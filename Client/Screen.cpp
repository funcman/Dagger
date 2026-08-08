#include "Screen.h"

#include <SDL3/SDL.h>

#include "Debug.h"

Screen::Screen(int width, int height, char const* title)
    : width_(width), height_(height), title_(title), window_(0), renderer_(0), texture_(0) {
    pixels_.resize((size_t)width_ * height_, 0);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        DDebugLog("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    window_ = SDL_CreateWindow(title_, width_, height_, 0);
    if (!window_) {
        DDebugLog("SDL_CreateWindow failed: %s", SDL_GetError());
        return;
    }

    // macOS: use the OpenGL backend. The Metal renderer (and SDL's software
    // fallback) synchronize SDL_RenderPresent to the display refresh rate on
    // this platform even with VSync disabled, capping windowed apps at ~120 Hz.
    // OpenGL with VSync off can present uncapped, giving the same throughput
    // as on Windows.
#if defined(__APPLE__)
    renderer_ = SDL_CreateRenderer(window_, "opengl");
#else
    renderer_ = SDL_CreateRenderer(window_, nullptr);
#endif
    if (!renderer_) {
        DDebugLog("SDL_CreateRenderer failed: %s", SDL_GetError());
        return;
    }

    // Disable VSync for uncapped frame rate.
    if (!SDL_SetRenderVSync(renderer_, SDL_RENDERER_VSYNC_DISABLED)) {
        DDebugLog("SDL_SetRenderVSync failed: %s", SDL_GetError());
    }

    // Upload the engine's 16-bit frame buffer as-is; SDL converts 16->32 while rendering
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB565,
                                 SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_) {
        DDebugLog("SDL_CreateTexture failed: %s", SDL_GetError());
        return;
    }

    fpsStart_ = SDL_GetTicks();
}

Screen::~Screen() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool Screen::ok() const {
    return texture_ != 0;
}

bool Screen::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            return false;
        }
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            return false;
        }
    }
    return true;
}

void Screen::present() {
    SDL_UpdateTexture(texture_, 0, pixels_.data(), width_ * (int)sizeof(uint16_t));
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, 0, 0);
    SDL_RenderPresent(renderer_);

    // Refresh the FPS counter in the window title once per second
    ++fpsFrames_;
    uint64_t now = SDL_GetTicks();
    uint64_t elapsed = now - fpsStart_;
    if (elapsed >= 1000) {
        char title[64];
        SDL_snprintf(title, sizeof(title), "%s - FPS: %u",
                     title_, (unsigned)(fpsFrames_ * 1000 / elapsed));
        SDL_SetWindowTitle(window_, title);
        fpsFrames_ = 0;
        fpsStart_ = now;
    }
}

void* Screen::frameBuffer() {
    return pixels_.data();
}

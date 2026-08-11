#include "PakCode.h"

#include "Debug.h"
#include "System.h"

#if defined(DAGGER_HAS_SDL3)
#include <SDL3/SDL.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

DPakCode* GpPakCode = nullptr;

namespace {

#if defined(DAGGER_HAS_SDL3)
struct DLibHandle {
    SDL_SharedObject* obj;
};
static DLibHandle* DLibOpen(char const* path) {
    SDL_SharedObject* obj = SDL_LoadObject(path);
    if (!obj)
        return nullptr;
    return new DLibHandle{obj};
}
static void DLibClose(DLibHandle* h) {
    if (!h)
        return;
    if (h->obj)
        SDL_UnloadObject(h->obj);
    delete h;
}
static void* DLibSym(DLibHandle* h, char const* name) {
    return h && h->obj ? SDL_LoadFunction(h->obj, name) : nullptr;
}
#elif defined(_WIN32)
struct DLibHandle {
    HMODULE obj;
};
static DLibHandle* DLibOpen(char const* path) {
    HMODULE obj = LoadLibraryA(path);
    if (!obj)
        return nullptr;
    return new DLibHandle{obj};
}
static void DLibClose(DLibHandle* h) {
    if (!h)
        return;
    if (h->obj)
        FreeLibrary(h->obj);
    delete h;
}
static void* DLibSym(DLibHandle* h, char const* name) {
    return h && h->obj ? (void*)GetProcAddress(h->obj, name) : nullptr;
}
#else
struct DLibHandle {
    void* obj;
};
static DLibHandle* DLibOpen(char const* path) {
    void* obj = dlopen(path, RTLD_NOW);
    if (!obj)
        return nullptr;
    return new DLibHandle{obj};
}
static void DLibClose(DLibHandle* h) {
    if (!h)
        return;
    if (h->obj)
        dlclose(h->obj);
    delete h;
}
static void* DLibSym(DLibHandle* h, char const* name) {
    return h && h->obj ? dlsym(h->obj, name) : nullptr;
}
#endif

}

DPakCode::DPakCode() : handle_(nullptr), encode_(nullptr), decode_(nullptr) {
    GpPakCode = this;
}

DPakCode::~DPakCode() {
    FreeLib();
}

bool DPakCode::LoadLib(char const* fileName) {
    if (handle_)
        return true;
    char path[DMAX_PATH];
    DPathGetFull(path, fileName);
    DLibHandle* h = DLibOpen(path);
    if (!h) {
        DDebugLog("DPakCode::LoadLib(%s) Fail", fileName);
        return false;
    }
    encode_ = (int (*)(unsigned char*, unsigned long*, unsigned char*, unsigned long))DLibSym(h, "compress");
    decode_ = (int (*)(unsigned char*, unsigned long*, unsigned char*, unsigned long))DLibSym(h, "uncompress");
    if (!encode_ || !decode_) {
        DLibClose(h);
        return false;
    }
    handle_ = h;
    return true;
}

void DPakCode::FreeLib() {
    DLibHandle* h = (DLibHandle*)handle_;
    if (h)
        DLibClose(h);
    handle_ = nullptr;
    encode_ = nullptr;
    decode_ = nullptr;
}

void DPakCode::Encode(DPakCodeInfo* info) {
    if (handle_ && encode_) {
        encode_(info->packBuf, &info->packLen, info->dataBuf, info->dataLen);
    }
}

void DPakCode::Decode(DPakCodeInfo* info) {
    if (handle_ && decode_) {
        decode_(info->dataBuf, &info->dataLen, info->packBuf, info->packLen);
    }
}

DWORD DPakCode::GetPackLen(DWORD dataLen) {
    return dataLen + dataLen / 10 + 1024;
}
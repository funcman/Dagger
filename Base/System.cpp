#include "System.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
// Must precede SDL.h: SDL pulls in <intrin.h>, which conflicts with winnt.h
// if it is included first.
#include <windows.h>
#endif
#if defined(DAGGER_HAS_SDL3)
#include <SDL3/SDL.h>
#endif
#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#if defined(DAGGER_HAS_SDL3)
SDL_Window* GpSdlWindow = nullptr;
#endif

char DRootPath[DMAX_PATH] = "/";
char DCurrPath[DMAX_PATH] = "/";

static char* rootPath_ = DRootPath;
static char* currPath_ = DCurrPath;
static bool rootInit_ = false;

static void DPathNormalize(char* path) {
    for (int i = 0; path[i]; ++i) {
        if (path[i] == '\\') path[i] = '/';
    }
}

static void DPathJoin(char* dest, char const* src) {
    int len = strlen(dest);
    if (len > 0 && dest[len - 1] == '/' && src[0] == '/') {
        strncat(dest, src + 1, DMAX_PATH - len - 1);
    } else {
        strncat(dest, src, DMAX_PATH - len - 1);
    }
}

static void DGetExecutablePath(char* out, int len) {
#if defined(_WIN32)
    GetModuleFileNameA(NULL, out, len);
#elif defined(__APPLE__)
    uint32_t size = len;
    _NSGetExecutablePath(out, &size);
#elif defined(__linux__)
    ssize_t n = readlink("/proc/self/exe", out, len - 1);
    if (n > 0) out[n] = 0;
#else
    out[0] = 0;
#endif
}

void DPathSetCurrent(char* path) {
    DPathNormalize(path);
    if (path[0] == '/' || (path[1] == ':' && path[0])) {
        DStrLCopy(currPath_, path, DMAX_PATH - 1);
    } else {
        currPath_[0] = '/';
        currPath_[1] = 0;
        strncat(currPath_, path, DMAX_PATH - 2);
    }
    int len = strlen(currPath_);
    if (len > 0 && currPath_[len - 1] != '/') {
        currPath_[len] = '/';
        currPath_[len + 1] = 0;
    }
}

void DPathGetFull(char* out, char const* file) {
    char temp[DMAX_PATH];
    DStrLCopy(temp, file, DMAX_PATH - 1);
    DPathNormalize(temp);
    if (!rootInit_) {
        char exe[DMAX_PATH];
        DGetExecutablePath(exe, DMAX_PATH);
        DPathExtractPath(rootPath_, exe);
        if (rootPath_[0] == 0) {
            rootPath_[0] = '/';
            rootPath_[1] = 0;
        }
        int len = strlen(rootPath_);
        if (len > 0 && rootPath_[len - 1] != '/') {
            rootPath_[len] = '/';
            rootPath_[len + 1] = 0;
        }
        rootInit_ = true;
    }
    if (temp[1] == ':' && temp[0]) {
        DStrLCopy(out, temp, DMAX_PATH - 1);
        return;
    }
    if (currPath_[1] == ':' && currPath_[0]) {
        strcpy(out, currPath_);
        DPathJoin(out, temp);
        return;
    }
    if (temp[0] == '/') {
        strcpy(out, rootPath_);
        DPathJoin(out, temp);
        return;
    }
    strcpy(out, rootPath_);
    DPathJoin(out, currPath_);
    DPathJoin(out, temp);
}

void DPathGetRelative(char* out, char const* file) {
    char temp[DMAX_PATH];
    DStrLCopy(temp, file, DMAX_PATH - 1);
    DPathNormalize(temp);
    if (temp[1] == ':' && temp[0]) {
        int rootLen = strlen(rootPath_);
        if (strncmp(temp, rootPath_, rootLen) == 0) {
            DStrLCopy(out, temp + rootLen, DMAX_PATH - 1);
        } else {
            DStrLCopy(out, temp, DMAX_PATH - 1);
        }
    } else if (temp[0] == '/') {
        DStrLCopy(out, temp, DMAX_PATH - 1);
    } else {
        strcpy(out, currPath_);
        DPathJoin(out, temp);
    }
    for (int i = 0; out[i]; ++i) {
        out[i] = (char)tolower(out[i]);
    }
}

bool DFileExists(char const* file) {
    char path[DMAX_PATH];
    DPathGetFull(path, file);
#if defined(_WIN32)
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES;
#elif defined(__linux__) || defined(__APPLE__)
    return access(path, F_OK) == 0;
#else
    return false;
#endif
}

void DPathChangeExt(char* file, char const* ext) {
    int i;
    for (i = 0; file[i]; ++i) {
        if (file[i] == '.') break;
    }
    if (file[i] == '.') {
        strcpy(&file[i], ext);
    } else {
        strncat(file, ext, DMAX_PATH - strlen(file) - 1);
    }
}

void DPathExtractName(char* out, char const* path) {
    out[0] = 0;
    int len = strlen(path);
    while (len > 0) {
        --len;
        if (path[len] == '/' || path[len] == '\\') {
            strcpy(out, &path[len + 1]);
            return;
        }
    }
}

void DPathExtractPath(char* out, char const* path) {
    DStrLCopy(out, path, DMAX_PATH - 1);
    int len = strlen(out);
    while (len > 0) {
        --len;
        if (out[len] == '/' || out[len] == '\\') {
            out[len] = 0;
            return;
        }
    }
}

void DPathGetHalf(char* out, char const* path) {
    // Mirrors GmGetHalfPath: strip absolute root prefix, prepend current path
    // when relative, then lowercase the result. Used as the key input for
    // DHashString, so Pack-time and runtime must agree exactly.
    if (path[1] == ':' && path[0]) {
        int rootLen = strlen(DRootPath);
        if (rootLen > 0 && strncmp(path, DRootPath, rootLen) == 0) {
            DStrLCopy(out, path + rootLen, DMAX_PATH - 1);
        } else {
            DStrLCopy(out, path, DMAX_PATH - 1);
        }
    } else if (path[0] == '\\' || path[0] == '/') {
        DStrLCopy(out, path, DMAX_PATH - 1);
    } else {
        DStrLCopy(out, DCurrPath, DMAX_PATH - 1);
        strncat(out, path, DMAX_PATH - strlen(out) - 1);
    }
    for (int i = 0; out[i]; ++i) {
        out[i] = (char)tolower((unsigned char)out[i]);
    }
}

DWORD DHashString(char const* str) {
    DWORD id = 0;
    for (int i = 0; str[i]; ++i) {
        id = (id + (i + 1) * (BYTE)str[i]) % 0x8000000b * 0xffffffef;
    }
    return id ^ 0x12345678;
}

DWORD DHashMemory(BYTE const* mem, int len) {
    DWORD id = 0;
    for (int i = 0; i < len; ++i) {
        id = (id + (i + 1) * mem[i]) % 0x8000000b * 0xffffffef;
    }
    return id ^ 0x12345678;
}

void DStrLCopy(char* dest, char const* src, int len) {
    if (dest && src) {
        strncpy(dest, src, len);
        dest[len] = 0;
    }
}

void DRandomSeed(UINT seed) {
    srand(seed);
}

long DRandom(int min, int max) {
    if (max < min) max = min;
    return min + rand() % (max - min + 1);
}

void DClipboardGetText(char* out, int len) {
    out[0] = 0;
#if defined(DAGGER_HAS_SDL3)
    char* p = SDL_GetClipboardText();
    if (p) {
        DStrLCopy(out, p, len - 1);
        SDL_free(p);
    }
#elif defined(_WIN32)
    if (!OpenClipboard(NULL)) return;
    HANDLE h = GetClipboardData(CF_TEXT);
    if (h) {
        char* p = (char*)GlobalLock(h);
        DStrLCopy(out, p, len - 1);
        GlobalUnlock(h);
    }
    CloseClipboard();
#endif
}

void DClipboardSetText(char const* text) {
#if defined(DAGGER_HAS_SDL3)
    SDL_SetClipboardText(text);
#elif defined(_WIN32)
    HANDLE h = GlobalAlloc(GHND, strlen(text) + 1);
    if (!h) return;
    char* p = (char*)GlobalLock(h);
    DStrLCopy(p, text, (int)strlen(text));
    GlobalUnlock(h);
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, h);
        CloseClipboard();
    } else {
        GlobalFree(h);
    }
#endif
}

void DIMEClose() {
#if defined(DAGGER_HAS_SDL3)
    if (GpSdlWindow) {
        SDL_StopTextInput(GpSdlWindow);
    }
#endif
}

void DIMEOpen() {
#if defined(DAGGER_HAS_SDL3)
    if (GpSdlWindow) {
        SDL_StartTextInput(GpSdlWindow);
    }
#endif
}

void DStrReplaceChar(char* str, char from, char to) {
    while (*str) {
        if (*str == from) *str = to;
        ++str;
    }
}

#ifndef DAGGER_SYSTEM_H
#define DAGGER_SYSTEM_H

#include "TypeDef.h"

const int DMAX_PATH = 260;

#if defined(DAGGER_HAS_SDL3)
struct SDL_Window;
extern SDL_Window* GpSdlWindow;
#endif

void DPathSetCurrent(char* path);
void DPathGetFull(char* out, char const* file);
void DPathGetRelative(char* out, char const* file);
bool DFileExists(char const* file);
void DPathChangeExt(char* file, char const* ext);
void DPathExtractName(char* out, char const* path);
void DPathExtractPath(char* out, char const* path);
void DPathGetHalf(char* out, char const* path);
DWORD DHashString(char const* str);
DWORD DHashMemory(BYTE const* mem, int len);
void DStrLCopy(char* dest, char const* src, int len);
void DRandomSeed(UINT seed);
long DRandom(int min, int max);
void DClipboardGetText(char* out, int len);
void DClipboardSetText(char const* text);
void DIMEClose();
void DIMEOpen();
void DStrReplaceChar(char* str, char from, char to);

extern char DRootPath[DMAX_PATH];
extern char DCurrPath[DMAX_PATH];

#endif

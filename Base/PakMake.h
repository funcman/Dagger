#ifndef DAGGER_PAKMAKE_H
#define DAGGER_PAKMAKE_H

#include "TypeDef.h"

struct DPakFileList {
    char* fileName;
    DWORD fileId;
};

struct DPakOffsList {
    DWORD fileId;
    DWORD fileOffset;
    DWORD fileLength;
    DWORD fileBlocks;
};

class DAGGER_API DPakMake {
public:
    bool Pack(char const* listFile, char const* packFile);
    bool UnPack(char const* datFile, char const* outPath);
};

#endif//DAGGER_PAKMAKE_H
#ifndef DAGGER_PAKCODE_H
#define DAGGER_PAKCODE_H

#include "TypeDef.h"

struct DPakCodeInfo {
    BYTE* dataBuf;
    DWORD dataLen;
    BYTE* packBuf;
    DWORD packLen;
};

class DAGGER_API DPakCode {
public:
    DPakCode();
    ~DPakCode();
    bool LoadLib(char const* fileName);
    void FreeLib();
    void Encode(DPakCodeInfo* info);
    void Decode(DPakCodeInfo* info);
    DWORD GetPackLen(DWORD dataLen);

private:
    void* handle_;
    int (*encode_)(unsigned char* dest, unsigned long* destLen, unsigned char* source, unsigned long sourceLen);
    int (*decode_)(unsigned char* dest, unsigned long* destLen, unsigned char* source, unsigned long sourceLen);
};

extern DAGGER_API DPakCode* GpPakCode;

#endif//DAGGER_PAKCODE_H
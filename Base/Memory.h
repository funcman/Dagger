#ifndef DAGGER_MEMORY_H
#define DAGGER_MEMORY_H

#include "TypeDef.h"

class DMemory {
public:
    void* memPtr;
    DWORD memLen;

public:
    DMemory();
    ~DMemory();
    void* Alloc(DWORD size);
    void Free();
    void Zero();
    void Fill(BYTE fill);
    void Copy(void* data, DWORD size);
    void* GetMemPtr() {
        return memPtr;
    };
    DWORD GetMemLen() {
        return memLen;
    };
};

#endif//DAGGER_MEMORY_H

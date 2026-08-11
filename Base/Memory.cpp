#include "Memory.h"
#include "MemPool.h"

#include <string.h>

DMemory::DMemory() {
    memPtr = NULL;
    memLen = 0;
}

DMemory::~DMemory() {
    Free();
}

void* DMemory::Alloc(DWORD size) {
    if (memPtr)
        Free();
    memLen = size;
    memPtr = DCAlloc(memLen);
    return memPtr;
}

void DMemory::Free() {
    DFree(memPtr);
    memPtr = NULL;
    memLen = 0;
}

void DMemory::Zero() {
    if (memPtr)
        memset(memPtr, 0, memLen);
}

void DMemory::Fill(BYTE fill) {
    if (memPtr)
        memset(memPtr, fill, memLen);
}

void DMemory::Copy(void* data, DWORD size) {
    Alloc(size);
    memcpy(memPtr, data, size);
}

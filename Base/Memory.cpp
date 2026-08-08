#include "Memory.h"
#include "MemPool.h"

#include <string.h>

DMemory::DMemory() {
    memPtr_ = NULL;
    memLen_ = 0;
}

DMemory::~DMemory() {
    Free();
}

void* DMemory::Alloc(DWORD size) {
    if (memPtr_)
        Free();
    memLen_ = size;
    memPtr_ = DCAlloc(memLen_);
    return memPtr_;
}

void DMemory::Free() {
    DFree(memPtr_);
    memPtr_ = NULL;
    memLen_ = 0;
}

void DMemory::Zero() {
    if (memPtr_)
        memset(memPtr_, 0, memLen_);
}

void DMemory::Fill(BYTE fill) {
    if (memPtr_)
        memset(memPtr_, fill, memLen_);
}

void DMemory::Copy(void* data, DWORD size) {
    Alloc(size);
    memcpy(memPtr_, data, size);
}

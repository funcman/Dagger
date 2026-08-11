#ifndef DAGGER_PAKNODE_H
#define DAGGER_PAKNODE_H

#include <mutex>

#include "BinFile.h"
#include "Memory.h"
#include "Node.h"

#include "TypeDef.h"

struct DPakHead {
    DWORD magicCode;
    DWORD fileNumber;
    DWORD fileOffset;
    DWORD listOffset;
};

struct DPakIndex {
    DWORD fileId;
    DWORD fileOffset;
    DWORD fileLength;
    DWORD fileBlocks;
};

class DPakNode : public DNode {
public:
    DBinFile zipFile;
    DMemory zipOffs;
    std::mutex mutex;
    DWORD fileNum;

public:
    DPakNode();
    ~DPakNode();
    void Init();
    void Free();
    bool Open(char const* fileName);
    DWORD Read(void* buffer, DWORD length);
    DWORD Seek(long offset, DWORD method);
    DWORD Tell();
    void Close();
    bool Find(char const* fileName, DPakIndex* index);
    void Lock() {
        mutex.lock();
    }
    void Unlock() {
        mutex.unlock();
    }
};

#endif//DAGGER_PAKNODE_H
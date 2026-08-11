#ifndef DAGGER_PAKFILE_H
#define DAGGER_PAKFILE_H

#include "BinFile.h"
#include "Memory.h"

#include "PakNode.h"
#include "TypeDef.h"

const int DPAK_DISK_FIRST = 0;
const int DPAK_PACK_FIRST = 1;

void DSetPakFileMode(int mode);

class DAGGER_API DPakFile {
public:
    DPakFile();
    ~DPakFile();
    bool Open(char const* fileName);
    void Close();
    DWORD Read(void* buffer, DWORD size);
    DWORD Seek(long offset, DWORD method);
    DWORD Tell();
    DWORD Size();
    bool Save(char const* fileName);

private:
    void ReadBlock_(BYTE* buf, int block);
    bool OpenPack_(char const* fileName);
    DWORD ReadPack_(void* buffer, DWORD size);
    DWORD SeekPack_(long offset, DWORD method);

    DBinFile file_;
    DMemory memFile_;
    DMemory memRead_;
    DMemory memBlock_;
    DPakNode* pakNode_;
    DWORD fileOfs_;
    DWORD fileLen_;
    DWORD filePtr_;
    DWORD dataPtr_;
    BYTE* buffer_;
    int blocks_;
    WORD* blockSizes_;
};

#endif//DAGGER_PAKFILE_H
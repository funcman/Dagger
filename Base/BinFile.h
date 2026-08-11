#ifndef DAGGER_BINFILE_H
#define DAGGER_BINFILE_H

#include <stdio.h>

#include "TypeDef.h"

const DWORD DFILE_BEGIN = 0;
const DWORD DFILE_CURRENT = 1;
const DWORD DFILE_END = 2;
const DWORD DFILE_SEEK_ERROR = 0xFFFFFFFF;

class DBinFile {
public:
    DBinFile();
    ~DBinFile();
    bool Open(char const* fileName);
    bool Update(char const* fileName);
    bool Create(char const* fileName);
    void Close();
    DWORD Read(void* buffer, DWORD bytes);
    DWORD Write(void const* buffer, DWORD bytes);
    DWORD Seek(long distance, DWORD method);
    DWORD Tell();
    DWORD Size();

private:
    FILE* handle_;
    DWORD size_;
    DWORD pointer_;
};

#endif//DAGGER_BINFILE_H

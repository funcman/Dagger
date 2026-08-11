#include "PakNode.h"

#include <string.h>

#include "Debug.h"
#include "MemPool.h"
#include "PakCode.h"
#include "System.h"

DPakNode::DPakNode() {
    Init();
}

DPakNode::~DPakNode() {
    Free();
}

void DPakNode::Init() {
    fileNum = 0;
}

void DPakNode::Free() {
    Close();
    zipOffs.Free();
}

bool DPakNode::Open(char const* fileName) {
    DPakHead header;
    DPakCodeInfo code;
    DWORD magic;

    if (fileNum)
        Close();

    if (!zipFile.Open(fileName))
        return false;

    if (zipFile.Read(&header, sizeof(header)) != sizeof(header))
        return false;

    if (header.magicCode != 0x12345678)
        return false;

    fileNum = header.fileNumber;

    zipFile.Seek(header.fileOffset, DFILE_BEGIN);
    if (zipFile.Read(&magic, sizeof(DWORD)) != sizeof(DWORD))
        return false;
    if (magic != 0x55AA55AA)
        return false;
    if (zipFile.Read(&code.packLen, sizeof(DWORD)) != sizeof(DWORD))
        return false;
    if (zipFile.Read(&code.dataLen, sizeof(DWORD)) != sizeof(DWORD))
        return false;
    if (code.dataLen != fileNum * sizeof(DPakIndex))
        return false;
    code.packBuf = (BYTE*)DCAlloc(code.packLen);
    if (zipFile.Read(code.packBuf, code.packLen) != code.packLen) {
        DFree(code.packBuf);
        return false;
    }
    zipOffs.Alloc(code.dataLen);
    code.dataBuf = (BYTE*)zipOffs.GetMemPtr();
    GpPakCode->Decode(&code);
    DFree(code.packBuf);
    return true;
}

DWORD DPakNode::Read(void* buffer, DWORD length) {
    return zipFile.Read(buffer, length);
}

DWORD DPakNode::Seek(long offset, DWORD method) {
    return zipFile.Seek(offset, method);
}

DWORD DPakNode::Tell() {
    return zipFile.Tell();
}

void DPakNode::Close() {
    zipFile.Close();
    zipOffs.Free();
    fileNum = 0;
}

bool DPakNode::Find(char const* fileName, DPakIndex* index) {
    char pathName[DMAX_PATH];

    if (fileNum <= 0)
        return false;
    DPakIndex* entries = (DPakIndex*)zipOffs.GetMemPtr();
    DPathGetHalf(pathName, fileName);
    DWORD id = DHashString(pathName);

    int begin = 0;
    int end = fileNum - 1;
    while (begin <= end) {
        int mid = (begin + end) / 2;
        if (id == entries[mid].fileId) {
            *index = entries[mid];
            return true;
        }
        if (id < entries[mid].fileId) {
            end = mid - 1;
        } else {
            begin = mid + 1;
        }
    }
    return false;
}
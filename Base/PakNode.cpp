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
    fileNum_ = 0;
}

void DPakNode::Free() {
    Close();
    zipOffs_.Free();
}

bool DPakNode::Open(char const* fileName) {
    DPakHead header;
    DPakCodeInfo code;
    DWORD magic;

    if (fileNum_) Close();

    if (!zipFile_.Open(fileName)) return false;

    if (zipFile_.Read(&header, sizeof(header)) != sizeof(header)) return false;

    if (header.magicCode != 0x12345678) return false;

    fileNum_ = header.fileNumber;

    zipFile_.Seek(header.fileOffset, DFILE_BEGIN);
    if (zipFile_.Read(&magic, sizeof(DWORD)) != sizeof(DWORD)) return false;
    if (magic != 0x55AA55AA) return false;
    if (zipFile_.Read(&code.packLen_, sizeof(DWORD)) != sizeof(DWORD)) return false;
    if (zipFile_.Read(&code.dataLen_, sizeof(DWORD)) != sizeof(DWORD)) return false;
    if (code.dataLen_ != fileNum_ * sizeof(DPakIndex)) return false;
    code.packBuf_ = (BYTE*)DCAlloc(code.packLen_);
    if (zipFile_.Read(code.packBuf_, code.packLen_) != code.packLen_) {
        DFree(code.packBuf_);
        return false;
    }
    zipOffs_.Alloc(code.dataLen_);
    code.dataBuf_ = (BYTE*)zipOffs_.GetMemPtr();
    GpPakCode->Decode(&code);
    DFree(code.packBuf_);
    return true;
}

DWORD DPakNode::Read(void* buffer, DWORD length) {
    return zipFile_.Read(buffer, length);
}

DWORD DPakNode::Seek(long offset, DWORD method) {
    return zipFile_.Seek(offset, method);
}

DWORD DPakNode::Tell() {
    return zipFile_.Tell();
}

void DPakNode::Close() {
    zipFile_.Close();
    zipOffs_.Free();
    fileNum_ = 0;
}

bool DPakNode::Find(char const* fileName, DPakIndex* index) {
    char pathName[DMAX_PATH];

    if (fileNum_ <= 0) return false;
    DPakIndex* entries = (DPakIndex*)zipOffs_.GetMemPtr();
    DPathGetHalf(pathName, fileName);
    DWORD id = DHashString(pathName);

    int begin = 0;
    int end = fileNum_ - 1;
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
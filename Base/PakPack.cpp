#include "PakPack.h"

#include <string.h>

#include "BinFile.h"
#include "Memory.h"
#include "PakCode.h"

namespace {

struct DPakSingleHead {
    DWORD magicId;
    DWORD fileLen;
    DWORD packLen;
    DWORD notUsed;
};

const DWORD PAK_MAGIC = 0x55AA55AA;

}

bool DPakPack::Pack(char const* inFile, char const* outFile) {
    DBinFile file;
    DMemory dataBuf;
    DMemory packBuf;
    DPakCodeInfo info;

    if (!GpPakCode) return false;
    if (!file.Open(inFile)) return false;
    DWORD size = file.Size();
    if (size == DFILE_SEEK_ERROR || size <= sizeof(DPakSingleHead)) return false;

    info.dataLen_ = size;
    info.packLen_ = GpPakCode->GetPackLen(info.dataLen_);
    if (!dataBuf.Alloc(info.dataLen_)) return false;
    if (!packBuf.Alloc(info.packLen_)) return false;

    file.Read(dataBuf.GetMemPtr(), info.dataLen_);
    file.Close();

    if (!file.Create(outFile)) return false;

    info.dataBuf_ = (BYTE*)dataBuf.GetMemPtr();
    info.packBuf_ = (BYTE*)packBuf.GetMemPtr();
    GpPakCode->Encode(&info);

    DPakSingleHead header;
    header.magicId = PAK_MAGIC;
    header.fileLen = info.dataLen_;
    header.packLen = info.packLen_;
    header.notUsed = 0;
    file.Write(&header, sizeof(header));
    file.Write(packBuf.GetMemPtr(), info.packLen_);
    file.Close();
    return true;
}

bool DPakPack::UnPack(char const* inFile, char const* outFile) {
    DBinFile file;
    DMemory packBuf;
    DMemory dataBuf;
    DPakCodeInfo info;

    if (!GpPakCode) return false;
    if (!file.Open(inFile)) return false;

    DPakSingleHead header;
    if (file.Read(&header, sizeof(header)) != sizeof(header)) return false;
    if (header.magicId != PAK_MAGIC) return false;

    info.packLen_ = header.packLen;
    info.dataLen_ = header.fileLen;
    if (!packBuf.Alloc(info.packLen_)) return false;
    if (!dataBuf.Alloc(info.dataLen_)) return false;

    file.Read(packBuf.GetMemPtr(), info.packLen_);
    file.Close();

    if (!file.Create(outFile)) return false;

    info.dataBuf_ = (BYTE*)dataBuf.GetMemPtr();
    info.packBuf_ = (BYTE*)packBuf.GetMemPtr();
    GpPakCode->Decode(&info);
    file.Write(dataBuf.GetMemPtr(), info.dataLen_);
    file.Close();
    return true;
}
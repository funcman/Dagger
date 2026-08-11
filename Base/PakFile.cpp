#include "PakFile.h"

#include <string.h>

#include "Debug.h"
#include "MemPool.h"
#include "PakCode.h"
#include "PakList.h"
#include "System.h"

namespace {

const DWORD BLOCK_SIZE = 0x10000;

int sPakFileMode = DPAK_DISK_FIRST;

}

void DSetPakFileMode(int mode) {
    sPakFileMode = mode;
}

DPakFile::DPakFile() : pakNode_(nullptr), fileOfs_(0), fileLen_(0), filePtr_(0), dataPtr_(0), buffer_(nullptr), blocks_(0), blockSizes_(nullptr) {}

DPakFile::~DPakFile() {
    Close();
}

bool DPakFile::Open(char const* fileName) {
    bool ok = false;
    Close();
    switch (sPakFileMode) {
    case DPAK_DISK_FIRST:
        ok = file_.Open(fileName);
        if (!ok)
            ok = OpenPack_(fileName);
        break;
    case DPAK_PACK_FIRST:
        ok = OpenPack_(fileName);
        if (!ok)
            ok = file_.Open(fileName);
        break;
    }
    return ok;
}

DWORD DPakFile::Read(void* buffer, DWORD size) {
    if (size == 0)
        return 0;
    if (pakNode_) {
        size = ReadPack_(buffer, size);
    } else {
        return file_.Read(buffer, size);
    }
    return size;
}

DWORD DPakFile::Seek(long offset, DWORD method) {
    if (pakNode_) {
        offset = SeekPack_(offset, method);
    } else {
        return file_.Seek(offset, method);
    }
    return offset;
}

DWORD DPakFile::Tell() {
    if (pakNode_)
        return filePtr_;
    return file_.Tell();
}

DWORD DPakFile::Size() {
    if (pakNode_)
        return fileLen_;
    return file_.Size();
}

void DPakFile::Close() {
    file_.Close();
    blockSizes_ = nullptr;
    buffer_ = nullptr;
    pakNode_ = nullptr;
}

bool DPakFile::OpenPack_(char const* fileName) {
    if (!GpPakList)
        return false;
    if (!fileName || !fileName[0])
        return false;

    DPakIndex index;
    pakNode_ = GpPakList->Find(fileName, &index);
    if (!pakNode_)
        return false;

    fileOfs_ = index.fileOffset;
    fileLen_ = index.fileLength;
    blocks_ = index.fileBlocks;

    if (!memFile_.Alloc(BLOCK_SIZE))
        return false;
    if (!memRead_.Alloc(BLOCK_SIZE))
        return false;
    if (!memBlock_.Alloc(blocks_ * 2))
        return false;

    buffer_ = (BYTE*)memFile_.GetMemPtr();
    blockSizes_ = (WORD*)memBlock_.GetMemPtr();

    pakNode_->Seek(fileOfs_, DFILE_BEGIN);
    pakNode_->Read(blockSizes_, blocks_ * 2);

    fileOfs_ = fileOfs_ + blocks_ * 2;
    dataPtr_ = fileOfs_;
    filePtr_ = 0;
    return true;
}

DWORD DPakFile::ReadPack_(void* buffer, DWORD size) {
    UINT block = 0;
    DWORD readSize = 0;
    DWORD blockPos = 0;
    BYTE* outBuf = (BYTE*)buffer;

    if (filePtr_ + size > fileLen_) {
        size = fileLen_ - filePtr_;
        readSize = size;
    } else {
        readSize = size;
    }

    block = filePtr_ >> 16;
    blockPos = filePtr_ & 0xffff;

    if (blockPos) {
        if (blockPos + size <= BLOCK_SIZE) {
            memcpy(outBuf, buffer_ + blockPos, size);
            filePtr_ += size;
            if ((filePtr_ & 0xffff) == 0) {
                dataPtr_ += (blockSizes_[block] == 0) ? BLOCK_SIZE : blockSizes_[block];
            }
            return size;
        }
        memcpy(outBuf, buffer_ + blockPos, BLOCK_SIZE - blockPos);
        outBuf += BLOCK_SIZE - blockPos;
        dataPtr_ += (blockSizes_[block] == 0) ? BLOCK_SIZE : blockSizes_[block];
        filePtr_ = (++block) << 16;
        size -= (BLOCK_SIZE - blockPos);
    }

    while (size > 0xffff) {
        ReadBlock_(outBuf, block);
        outBuf += BLOCK_SIZE;
        dataPtr_ += (blockSizes_[block] == 0) ? BLOCK_SIZE : blockSizes_[block];
        filePtr_ = (++block) << 16;
        size -= BLOCK_SIZE;
    }

    if (size == 0)
        return readSize;

    ReadBlock_(buffer_, block);
    memcpy(outBuf, buffer_, size);
    filePtr_ += size;
    return readSize;
}

DWORD DPakFile::SeekPack_(long offset, DWORD method) {
    if (!pakNode_)
        return file_.Seek(offset, method);

    int filePtr = filePtr_;

    switch (method) {
    case DFILE_BEGIN:
        filePtr = offset;
        break;
    case DFILE_END:
        filePtr = fileLen_ + offset;
        break;
    case DFILE_CURRENT:
        filePtr = filePtr_ + offset;
        break;
    }

    if (filePtr < 0) {
        filePtr = 0;
    } else if (filePtr > (int)fileLen_) {
        filePtr = fileLen_;
    }

    filePtr_ = filePtr;
    dataPtr_ = fileOfs_;

    int fullBlocks = filePtr >> 16;
    for (int i = 0; i < fullBlocks; i++) {
        dataPtr_ += (blockSizes_[i] == 0) ? BLOCK_SIZE : blockSizes_[i];
    }
    if (filePtr & 0xffff) {
        ReadBlock_(buffer_, fullBlocks);
    }
    return filePtr_;
}

bool DPakFile::Save(char const* fileName) {
    if (!pakNode_)
        return true;

    if (!file_.Create(fileName))
        return false;

    DWORD size = fileLen_;
    int block = 0;

    dataPtr_ = fileOfs_;

    while (size > BLOCK_SIZE) {
        ReadBlock_(buffer_, block);
        size -= BLOCK_SIZE;
        file_.Write(buffer_, BLOCK_SIZE);
        dataPtr_ += (blockSizes_[block] == 0) ? BLOCK_SIZE : blockSizes_[block];
        block++;
    }

    ReadBlock_(buffer_, block);
    file_.Write(buffer_, size);
    file_.Close();
    return true;
}

void DPakFile::ReadBlock_(BYTE* buf, int block) {
    DPakCodeInfo info;
    info.packBuf = (BYTE*)memRead_.GetMemPtr();
    info.packLen = blockSizes_[block];
    info.dataBuf = buf;
    info.dataLen = BLOCK_SIZE;

    if (info.packLen == 0) {
        pakNode_->Seek(dataPtr_, DFILE_BEGIN);
        pakNode_->Read(info.dataBuf, info.dataLen);
        return;
    }

    if (block == (blocks_ - 1)) {
        info.dataLen = fileLen_ - block * BLOCK_SIZE;
    }

    pakNode_->Seek(dataPtr_, DFILE_BEGIN);
    pakNode_->Read(info.packBuf, info.packLen);
    pakNode_->Lock();
    GpPakCode->Decode(&info);
    pakNode_->Unlock();
}
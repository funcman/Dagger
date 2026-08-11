#include "BinFile.h"

#include <stdio.h>

#include "Debug.h"
#include "System.h"

DBinFile::DBinFile() : handle_(nullptr), size_(0), pointer_(0) {}

DBinFile::~DBinFile() {
    Close();
}

bool DBinFile::Open(char const* fileName) {
    Close();
    if (!fileName || !fileName[0]) return false;
    char path[DMAX_PATH];
    DPathGetFull(path, fileName);
    handle_ = fopen(path, "rb");
    if (!handle_) {
        DDebugLog("DBinFile::Open(%s) Fail", fileName);
        return false;
    }
    return true;
}

bool DBinFile::Update(char const* fileName) {
    Close();
    if (!fileName || !fileName[0]) return false;
    char path[DMAX_PATH];
    DPathGetFull(path, fileName);
    handle_ = fopen(path, "r+b");
    if (!handle_) {
        DDebugLog("DBinFile::Update(%s) Fail", fileName);
        return false;
    }
    return true;
}

bool DBinFile::Create(char const* fileName) {
    Close();
    if (!fileName || !fileName[0]) return false;
    char path[DMAX_PATH];
    DPathGetFull(path, fileName);
    handle_ = fopen(path, "wb");
    if (!handle_) {
        DDebugLog("DBinFile::Create(%s) Fail", fileName);
        return false;
    }
    return true;
}

void DBinFile::Close() {
    if (handle_) {
        fclose(handle_);
    }
    handle_ = nullptr;
    size_ = 0;
    pointer_ = 0;
}

DWORD DBinFile::Read(void* buffer, DWORD bytes) {
    if (!handle_ || bytes == 0) return 0;
    DWORD read = (DWORD)fread(buffer, 1, bytes, handle_);
    pointer_ += read;
    return read;
}

DWORD DBinFile::Write(void const* buffer, DWORD bytes) {
    if (!handle_ || bytes == 0) return 0;
    DWORD written = (DWORD)fwrite(buffer, 1, bytes, handle_);
    pointer_ += written;
    return written;
}

DWORD DBinFile::Seek(long distance, DWORD method) {
    if (!handle_) return DFILE_SEEK_ERROR;
    if (fseek(handle_, distance, method) != 0) return DFILE_SEEK_ERROR;
    pointer_ = (DWORD)ftell(handle_);
    return pointer_;
}

DWORD DBinFile::Tell() {
    if (!handle_) return DFILE_SEEK_ERROR;
    return pointer_;
}

DWORD DBinFile::Size() {
    if (!handle_) return DFILE_SEEK_ERROR;
    if (size_ == 0) {
        long pos = ftell(handle_);
        fseek(handle_, 0, DFILE_END);
        size_ = (DWORD)ftell(handle_);
        fseek(handle_, pos, DFILE_BEGIN);
    }
    return size_;
}

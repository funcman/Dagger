#include "TxtFile.h"

#include <string.h>

#include "Debug.h"
#include "MemPool.h"
#include "PakFile.h"

DTxtFile::DTxtFile() : currLine_(nullptr), size_(0), lineCount_(0) {}

DTxtFile::~DTxtFile() {
    Close();
}

bool DTxtFile::Open(char const* fileName) {
    if (!fileName || !fileName[0])
        return false;
    Close();
    DPakFile file;
    if (!file.Open(fileName))
        return false;
    DWORD size = file.Size();
    if (size == DFILE_SEEK_ERROR)
        return false;
    memFile_.Alloc(size + 2);
    if (size > 0)
        file.Read(memFile_.GetMemPtr(), size);
    size_ = size;
    ReadAllLines_();
    return true;
}

void DTxtFile::OpenMem(void* buffer, DWORD length) {
    Close();
    memFile_.Alloc(length + 2);
    if (buffer && length > 0)
        memcpy(memFile_.GetMemPtr(), buffer, length);
    size_ = length;
    ReadAllLines_();
}

void DTxtFile::ReadAllLines_() {
    char* pHead = (char*)memFile_.GetMemPtr();
    char* pTail = pHead + size_;
    // A single '\n' sentinel at pTail[0] guarantees the final line is picked
    // up even when the source file lacks a trailing newline, regardless of
    // whether the file uses LF, CRLF, or bare CR line endings.
    pTail[0] = '\n';
    lineCount_ = 0;
    while (pHead < pTail) {
        DTxtLine* line = (DTxtLine*)DCAlloc(sizeof(DTxtLine));
        line->line = pHead;
        lineList_.AddTail(line);
        pHead = GotoNextLine_(pHead, pTail);
        lineCount_++;
    }
}

char* DTxtFile::GotoNextLine_(char* pHead, char* pTail) {
    while (pHead < pTail && *pHead != '\r' && *pHead != '\n') {
        pHead++;
    }
    if (pHead < pTail && *pHead == '\r') {
        *pHead = 0;
        pHead++;
    }
    if (pHead < pTail && *pHead == '\n') {
        *pHead = 0;
        pHead++;
    }
    return pHead;
}

void DTxtFile::Close() {
    lineList_.Free();
    currLine_ = nullptr;
    size_ = 0;
    lineCount_ = 0;
}

char* DTxtFile::FirstLine() {
    currLine_ = (DTxtLine*)lineList_.GetHead();
    return currLine_ ? currLine_->line : nullptr;
}

char* DTxtFile::NextLine() {
    if (!currLine_)
        return nullptr;
    currLine_ = (DTxtLine*)currLine_->GetNext();
    return currLine_ ? currLine_->line : nullptr;
}

char* DTxtFile::CurrLine() {
    return currLine_ ? currLine_->line : nullptr;
}
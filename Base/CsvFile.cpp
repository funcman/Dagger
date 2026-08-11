#include "CsvFile.h"

#include "BinFile.h"
#include "Debug.h"
#include "MemPool.h"

DCsvFile::DCsvFile() : csvBuf_(nullptr) {}

DCsvFile::~DCsvFile() {
    Free();
}

bool DCsvFile::Load(char const* fileName) {
    Free();
    DBinFile file;
    if (!file.Open(fileName)) return false;
    DWORD size = file.Size();
    if (size == DFILE_SEEK_ERROR || size == 0) return false;
    csvBuf_ = (char*)DCAlloc(size + 1);
    if (!csvBuf_) return false;
    file.Read(csvBuf_, size);
    csvBuf_[size] = 0;
    rows_.push_back(&csvBuf_[0]);
    for (DWORD i = 0; i < size; ++i) {
        if (csvBuf_[i] == '\n') {
            csvBuf_[i] = 0;
            rows_.push_back(&csvBuf_[i + 1]);
        }
        // '\r' is intentionally left in place: GetStr treats it as a column
        // terminator, matching the legacy KmCsvFile behavior used for CR-only
        // and CRLF input.
    }
    return true;
}

void DCsvFile::Free() {
    if (csvBuf_) {
        DFree(csvBuf_);
        csvBuf_ = nullptr;
    }
    rows_.clear();
}

bool DCsvFile::GetStr(int row, int col, char* buf) {
    if (row < 0 || row >= (int)rows_.size() || !buf) return false;
    char* p = rows_[row];
    if (!p) return false;
    int currentCol = 0;
    bool inQuote = false;
    while (currentCol < col && p[0]) {
        if (p[0] == '"') {
            inQuote = !inQuote;
            p++;
        } else if (p[0] == '\\' && p[1] == '"') {
            p += 2;
        } else if (p[0] == '\n') {
            return false;
        } else if (p[0] == ',' && !inQuote) {
            currentCol++;
            p++;
        } else {
            p++;
        }
    }
    while (p[0]) {
        if (p[0] == '"') {
            p++;
        } else if (p[0] == '\\' && p[1] == '"') {
            *buf = '"';
            p += 2;
            buf++;
        } else if (p[0] == ',' || p[0] == '\r' || p[0] == '\n') {
            *buf = 0;
            return true;
        } else {
            *buf = p[0];
            p++;
            buf++;
        }
    }
    *buf = 0;
    return true;
}

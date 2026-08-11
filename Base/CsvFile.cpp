#include "CsvFile.h"

#include "BinFile.h"
#include "Debug.h"
#include "MemPool.h"

DCsvFile::DCsvFile() : csvBuf_(nullptr), sepChar_(',') {}

DCsvFile::~DCsvFile() {
    Free();
}

bool DCsvFile::Load(char const* fileName, DSep sep) {
    Free();
    sepChar_ = (sep == DSepSemicolon) ? ';' : (sep == DSepTab) ? '\t' : ',';
    DBinFile file;
    if (!file.Open(fileName)) return false;
    DWORD size = file.Size();
    if (size == DFILE_SEEK_ERROR || size == 0) return false;
    csvBuf_ = (char*)DCAlloc(size + 1);
    if (!csvBuf_) return false;
    file.Read(csvBuf_, size);
    csvBuf_[size] = 0;

    // Strip UTF-8 BOM transparently so the first column name is not
    // prefixed with three invisible bytes.
    if (size >= 3
        && (BYTE)csvBuf_[0] == 0xEF
        && (BYTE)csvBuf_[1] == 0xBB
        && (BYTE)csvBuf_[2] == 0xBF) {
        memmove(csvBuf_, csvBuf_ + 3, size - 3 + 1);
        size -= 3;
    }

    rows_.push_back(&csvBuf_[0]);
    // Walk the buffer tracking quote state so that '\n' inside a quoted
    // field does not split rows (RFC 4180 multi-line field). The "" escape
    // is honored here so an embedded "" cannot accidentally close the quote.
    // '\r' is intentionally left in place: GetStr treats it as a column
    // terminator, matching the legacy KmCsvFile behavior used for CR-only
    // and CRLF input.
    bool inQuote = false;
    for (DWORD i = 0; i < size; ++i) {
        char c = csvBuf_[i];
        if (c == '\\' && i + 1 < size && csvBuf_[i + 1] == '"') {
            // Legacy \" escape: a literal quote that never toggles quote
            // state. Must be honored here too, otherwise a \" inside a
            // quoted field would close (and reopen) the quote and desync
            // row splitting from GetStr's field parsing.
            i++;
        } else if (inQuote) {
            if (c == '"') {
                if (i + 1 < size && csvBuf_[i + 1] == '"') {
                    i++;  // RFC 4180: "" inside a quoted field is a literal "
                } else {
                    inQuote = false;  // closing quote
                }
            }
            // any other byte (including \r, \n) is data inside the field
        } else {
            if (c == '"') {
                inQuote = true;  // opening quote
            } else if (c == '\n') {
                csvBuf_[i] = 0;
                if (i + 1 < size) rows_.push_back(&csvBuf_[i + 1]);
            }
        }
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

    bool inQuote = false;  // currently between opening " and closing "
    bool quoted = false;    // the current field started with "

    // Skip phase: advance past `col` columns.
    while (col > 0 && p[0]) {
        if (!quoted && p[0] == '"') {
            quoted = true;
            inQuote = true;
            p++;
        } else if (quoted && inQuote && p[0] == '"') {
            if (p[1] == '"') {
                p += 2;  // RFC 4180: "" inside a quoted field is a literal "
            } else {
                inQuote = false;  // closing quote
                p++;
            }
        } else if (p[0] == '\\' && p[1] == '"') {
            p += 2;  // Legacy \" escape, works inside or outside quoted fields.
        } else if (!inQuote && p[0] == '\n') {
            return false;
        } else if (!inQuote && p[0] == sepChar_) {
            col--;
            quoted = false;
            p++;
        } else {
            p++;
        }
    }
    if (col > 0) return false;

    // Copy phase: extract the column value into buf.
    while (p[0]) {
        if (!quoted && p[0] == '"') {
            quoted = true;
            inQuote = true;
            p++;
        } else if (quoted && inQuote && p[0] == '"') {
            if (p[1] == '"') {
                *buf = '"';
                p += 2;
                buf++;
            } else {
                inQuote = false;
                p++;
            }
        } else if (p[0] == '\\' && p[1] == '"') {
            *buf = '"';
            p += 2;
            buf++;
        } else if (!inQuote && (p[0] == sepChar_ || p[0] == '\r' || p[0] == '\n')) {
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

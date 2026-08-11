#ifndef DAGGER_CSVFILE_H
#define DAGGER_CSVFILE_H

#include <vector>

#include "TypeDef.h"

class DCsvFile {
public:
    // Field separator used by Load/GetStr. DSepComma keeps the classic
    // CSV behavior; the other two are for semicolon- or tab-delimited
    // files. Quoting rules are identical for all three.
    enum DSep {
        DSepComma = 0,
        DSepSemicolon,
        DSepTab
    };

    DCsvFile();
    ~DCsvFile();
    bool Load(char const* fileName, DSep sep = DSepComma);
    void Free();
    bool GetStr(int row, int col, char* buf);
    int GetRowCount() const {
        return (int)rows_.size();
    };

private:
    char* csvBuf_;
    char sepChar_;
    std::vector<char*> rows_;
};

#endif//DAGGER_CSVFILE_H

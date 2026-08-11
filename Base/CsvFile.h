#ifndef DAGGER_CSVFILE_H
#define DAGGER_CSVFILE_H

#include <vector>

#include "TypeDef.h"

class DCsvFile {
public:
    DCsvFile();
    ~DCsvFile();
    bool Load(char const* fileName);
    void Free();
    bool GetStr(int row, int col, char* buf);

private:
    char* csvBuf_;
    std::vector<char*> rows_;
};

#endif//DAGGER_CSVFILE_H

#ifndef DAGGER_TXTFILE_H
#define DAGGER_TXTFILE_H

#include "List.h"
#include "Memory.h"

#include "TypeDef.h"

class DTxtLine : public DNode {
public:
    char* line;
};

class DAGGER_API DTxtFile {
public:
    DTxtFile();
    ~DTxtFile();
    bool Open(char const* fileName);
    void OpenMem(void* buffer, DWORD length);
    void Close();
    char* FirstLine();
    char* NextLine();
    char* CurrLine();
    int GetLineCount() {
        return lineCount_;
    };

private:
    void ReadAllLines_();
    char* GotoNextLine_(char* head, char* tail);

    DMemory memFile_;
    DList lineList_;
    DTxtLine* currLine_;
    DWORD size_;
    int lineCount_;
};

#endif//DAGGER_TXTFILE_H
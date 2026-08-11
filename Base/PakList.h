#ifndef DAGGER_PAKLIST_H
#define DAGGER_PAKLIST_H

#include "List.h"

#include "PakNode.h"

class DPakList : public DList {
public:
    DPakList();
    ~DPakList();
    bool Open(char const* fileName);
    bool Patch(char const* fileName);
    DPakNode* Find(char const* fileName, DPakIndex* index);
    void Free();

    DList patchList;
};

extern DAGGER_API DPakList* GpPakList;

#endif//DAGGER_PAKLIST_H
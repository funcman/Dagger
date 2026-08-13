#ifndef DAGGER_FINDLIST_H
#define DAGGER_FINDLIST_H

#include "List.h"
#include "TypeDef.h"

class DFindList : public DList {
public:
    DFindList();
    ~DFindList();
    void Init(int hashSize);
    void Free();
    void AddName(char const* name, void* ptr);
    void DelName(char const* name);
    void* GetName(char const* name);

private:
    UINT HashKey(char const* name);

    int hashSize_;
    DList* buckets_;
};

#endif//DAGGER_FINDLIST_H

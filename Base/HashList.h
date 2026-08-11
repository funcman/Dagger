#ifndef DAGGER_HASHLIST_H
#define DAGGER_HASHLIST_H

#include "List.h"
#include "TypeDef.h"

const int DHASH_SIZE = 100;

class DNameNode : public DNode {
public:
    char name_[80];
    DNode* hashNode_;
    void SetName(char const* name);
    void Remove();
    void Delete();
};

class DHashList : public DList {
public:
    ~DHashList();
    void Free();
    void AddHead(DNameNode* node);
    void AddTail(DNameNode* node);
    void Changed(DNameNode* node);
    DNameNode* RemoveHead();
    DNameNode* RemoveTail();
    DNameNode* FindName(char const* name);
    UINT HashKey(char const* name);

private:
    DList buckets_[DHASH_SIZE];
};

#endif//DAGGER_HASHLIST_H

#include "FindList.h"

#include <string.h>

#include "HashList.h"
#include "MemPool.h"

struct FindNode : public DNameNode {
    void* ptr;
};

struct FindHashNode : public DNode {
    FindNode* node;
};

DFindList::DFindList() : hashSize_(0), buckets_(nullptr) {}

DFindList::~DFindList() {
    Free();
}

void DFindList::Init(int hashSize) {
    Free();
    if (hashSize > 0) {
        hashSize_ = hashSize;
        buckets_ = (DList*)DCAlloc(sizeof(DList) * hashSize_);
    }
}

void DFindList::Free() {
    for (int i = 0; i < hashSize_; i++) {
        buckets_[i].Free();
    }
    DFree(buckets_);
    buckets_ = nullptr;
    hashSize_ = 0;
    DList::Free();
}

void DFindList::AddName(char const* name, void* ptr) {
    if (!buckets_)
        Init(DHASH_SIZE);
    int i = HashKey(name) % hashSize_;
    DNode* p = buckets_[i].GetHead();
    while (p) {
        FindHashNode* hashNode = (FindHashNode*)p;
        if (strcmp(hashNode->node->name, name) == 0) {
            hashNode->node->ptr = ptr;
            return;
        }
        p = p->GetNext();
    }
    FindNode* node = (FindNode*)DCAlloc(sizeof(FindNode));
    node->SetName(name);
    node->ptr = ptr;
    AddTail(node);
    FindHashNode* hashNode = (FindHashNode*)DCAlloc(sizeof(FindHashNode));
    hashNode->node = node;
    node->hashNode = hashNode;
    buckets_[i].AddTail(hashNode);
}

void DFindList::DelName(char const* name) {
    if (!buckets_)
        return;
    int i = HashKey(name) % hashSize_;
    DNode* p = buckets_[i].GetHead();
    while (p) {
        FindHashNode* hashNode = (FindHashNode*)p;
        if (strcmp(hashNode->node->name, name) == 0) {
            hashNode->node->Delete();
            break;
        }
        p = p->GetNext();
    }
}

void* DFindList::GetName(char const* name) {
    if (!buckets_)
        return nullptr;
    int i = HashKey(name) % hashSize_;
    DNode* p = buckets_[i].GetHead();
    while (p) {
        FindHashNode* hashNode = (FindHashNode*)p;
        if (strcmp(hashNode->node->name, name) == 0)
            return hashNode->node->ptr;
        p = p->GetNext();
    }
    return nullptr;
}

UINT DFindList::HashKey(char const* name) {
    UINT hash = 0;
    while (*name) {
        hash = (hash << 5) + hash + (BYTE)*name;
        name++;
    }
    return hash;
}

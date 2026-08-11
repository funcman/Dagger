#include "HashList.h"

#include <string.h>

#include "MemPool.h"
#include "System.h"

struct HashNode : public DNode {
    DNameNode* node;
};

void DNameNode::SetName(char const* name) {
    DStrLCopy(name_, name, 79);
    name_[79] = 0;
}

void DNameNode::Remove() {
    if (hashNode_) {
        hashNode_->Remove();
        DFree(hashNode_);
        hashNode_ = nullptr;
    }
    DNode::Remove();
}

void DNameNode::Delete() {
    Remove();
    DFree(this);
}

DHashList::~DHashList() {
    Free();
}

void DHashList::Free() {
    for (int i = 0; i < DHASH_SIZE; i++) {
        buckets_[i].Free();
    }
    DList::Free();
}

void DHashList::AddHead(DNameNode* node) {
    HashNode* hashNode = (HashNode*)DCAlloc(sizeof(HashNode));
    hashNode->node = node;
    node->hashNode_ = hashNode;
    int i = HashKey(node->name_) % DHASH_SIZE;
    buckets_[i].AddTail(hashNode);
    DList::AddHead(node);
}

void DHashList::AddTail(DNameNode* node) {
    HashNode* hashNode = (HashNode*)DCAlloc(sizeof(HashNode));
    hashNode->node = node;
    node->hashNode_ = hashNode;
    int i = HashKey(node->name_) % DHASH_SIZE;
    buckets_[i].AddTail(hashNode);
    DList::AddTail(node);
}

void DHashList::Changed(DNameNode* node) {
    node->hashNode_->Remove();
    int i = HashKey(node->name_) % DHASH_SIZE;
    buckets_[i].AddTail(node->hashNode_);
}

DNameNode* DHashList::RemoveHead() {
    DNameNode* node = (DNameNode*)GetHead();
    if (node) {
        node->Remove();
    }
    return node;
}

DNameNode* DHashList::RemoveTail() {
    DNameNode* node = (DNameNode*)GetTail();
    if (node) {
        node->Remove();
    }
    return node;
}

DNameNode* DHashList::FindName(char const* name) {
    int i = HashKey(name) % DHASH_SIZE;
    DNode* p = buckets_[i].GetHead();
    while (p) {
        HashNode* hashNode = (HashNode*)p;
        if (strcmp(hashNode->node->name_, name) == 0) {
            return hashNode->node;
        }
        p = p->GetNext();
    }
    return nullptr;
}

UINT DHashList::HashKey(char const* name) {
    UINT hash = 0;
    while (*name) {
        hash = (hash << 5) + hash + (BYTE)*name;
        name++;
    }
    return hash;
}

#include "StrList.h"

#include <string.h>

#include "MemPool.h"
#include "System.h"

namespace {

DStrNode* NewStrNode(char const* name) {
    DStrNode* node = (DStrNode*)DCAlloc(sizeof(DStrNode));
    // DCAlloc is malloc-like and recycles memory, so the DNode link
    // fields must be initialized by hand; the constructor never runs.
    node->prev = nullptr;
    node->next = nullptr;
    node->SetThis();
    node->SetName(name);
    return node;
}

}

void DStrNode::SetName(char const* name) {
    DStrLCopy(this->name, name, sizeof(this->name) - 1);
}

DStrNode* DStrList::FindName(char const* name) {
    DStrNode* node = (DStrNode*)GetHead();
    while (node) {
        if (strcmp(name, node->GetName()) == 0) {
            return node;
        }
        node = (DStrNode*)node->GetNext();
    }
    return nullptr;
}

DStrNode* DStrList::AddHeadName(char const* name) {
    DStrNode* node = NewStrNode(name);
    AddHead(node);
    return node;
}

DStrNode* DStrList::AddTailName(char const* name) {
    DStrNode* node = NewStrNode(name);
    AddTail(node);
    return node;
}

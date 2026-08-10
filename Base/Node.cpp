#include "Node.h"

#include "MemPool.h"

#include <cstdint>

DNode::DNode() {
    next = nullptr;
    prev = nullptr;
    thisPtr = 0;
}

bool DNode::IsValid() {
    return (thisPtr == ((uintptr_t)prev + (uintptr_t)next));
}

void DNode::SetThis() {
    thisPtr = ((uintptr_t)prev + (uintptr_t)next);
}

DNode* DNode::GetPrev() {
    if (prev && IsValid()) {
        if (prev->IsValid() && prev->prev)
            return prev;
    }
    return nullptr;
}

DNode* DNode::GetNext() {
    if (next && IsValid()) {
        if (next->IsValid() && next->next)
            return next;
    }
    return nullptr;
}

bool DNode::InsertBefore(DNode* Node) {
    if (prev && IsValid()) {
        if (prev->IsValid() && Node->IsValid()) {
            Node->Remove();
            Node->prev = prev;
            Node->next = this;
            Node->SetThis();
            prev->next = Node;
            prev->SetThis();
            prev = Node;
            SetThis();
            return true;
        }
    }
    return false;
}

bool DNode::InsertAfter(DNode* Node) {
    if (next && IsValid()) {
        if (next->IsValid() && Node->IsValid()) {
            Node->Remove();
            Node->prev = this;
            Node->next = next;
            Node->SetThis();
            next->prev = Node;
            next->SetThis();
            next = Node;
            SetThis();
            return true;
        }
    }
    return false;
}

bool DNode::Remove() {
    if (prev && next && IsValid()) {
        if (prev->IsValid() && next->IsValid()) {
            prev->next = next;
            prev->SetThis();
            next->prev = prev;
            next->SetThis();
            prev = nullptr;
            next = nullptr;
            SetThis();
            return true;
        }
    }
    return false;
}

void DNode::Delete() {
    Remove();
    DFree(this);
}

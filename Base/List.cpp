#include "List.h"

#include "MemPool.h"

DList::DList() {
    Init();
}

DList::~DList() {
    Free();
}

void DList::Init() {
    head_.prev = nullptr;
    head_.next = &tail_;
    head_.SetThis();
    tail_.prev = &head_;
    tail_.next = nullptr;
    tail_.SetThis();
}

void DList::Free() {
    while (DNode* pNode = RemoveHead()) {
        DFree(pNode);
    }
}

bool DList::IsEmpty() {
    return (head_.GetNext() == nullptr);
}

DNode* DList::GetHead() {
    return head_.GetNext();
}

DNode* DList::GetTail() {
    return tail_.GetPrev();
}

void DList::AddHead(DNode* pNode) {
    if (head_.next == nullptr)
        Init();
    head_.InsertAfter(pNode);
}

void DList::AddTail(DNode* pNode) {
    if (tail_.prev == nullptr)
        Init();
    tail_.InsertBefore(pNode);
}

DNode* DList::RemoveHead() {
    DNode* pNode = head_.GetNext();
    if (pNode) {
        pNode->Remove();
    }
    return pNode;
}

DNode* DList::RemoveTail() {
    DNode* pNode = tail_.GetPrev();
    if (pNode) {
        pNode->Remove();
    }
    return pNode;
}

int DList::GetCount() {
    int nNodeNum = 0;
    DNode* pNode = GetHead();
    while (pNode) {
        pNode = pNode->GetNext();
        nNodeNum++;
    }
    return nNodeNum;
}

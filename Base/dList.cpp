#include "dList.h"
#include "dBuffer.h"

dList::dList() {
    Init();
}

dList::~dList() {
    Free();
}

void dList::Init() {
    head_.SetPrev(0);
    head_.SetNext(&tail_);
    head_.SetThis();
    tail_.SetPrev(&head_);
    tail_.SetNext(0);
    tail_.SetThis();
}

void dList::Free() {
    while (dNode* node = RemoveHead()) {
        dFree(node);
    }
}

bool dList::IsEmpty() {
    return head_.GetNext() == 0;
}

dNode* dList::GetHead() {
    return head_.GetNext();
}

dNode* dList::GetTail() {
    return tail_.GetPrev();
}

void dList::AddHead(dNode* node) {
    if (head_.GetNext() == 0) {
        Init();
    }
    head_.InsertAfter(node);
}

void dList::AddTail(dNode* node) {
    if (tail_.GetPrev() == 0) {
        Init();
    }
    tail_.InsertBefore(node);
}

dNode* dList::RemoveHead() {
    dNode* node = head_.GetNext();
    if (node) {
        node->Remove();
    }
    return node;
}

dNode* dList::RemoveTail() {
    dNode* node = tail_.GetPrev();
    if (node) {
        node->Remove();
    }
    return node;
}

long dList::GetCount() {
    long count = 0;
    dNode* node = GetHead();
    while (node) {
        node = node->GetNext();
        ++count;
    }
    return count;
}

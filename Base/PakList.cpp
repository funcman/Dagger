#include "PakList.h"

#include "MemPool.h"

DPakList* GpPakList = nullptr;

DPakList::DPakList() {
    GpPakList = this;
}

DPakList::~DPakList() {
    Free();
}

bool DPakList::Open(char const* fileName) {
    DPakNode* node = (DPakNode*)DCAlloc(sizeof(DPakNode));
    node->Init();
    if (node->Open(fileName)) {
        AddHead(node);
        return true;
    }
    DFree(node);
    return false;
}

bool DPakList::Patch(char const* fileName) {
    DPakNode* node = (DPakNode*)DCAlloc(sizeof(DPakNode));
    node->Init();
    if (node->Open(fileName)) {
        patchList_.AddHead(node);
        return true;
    }
    DFree(node);
    return false;
}

DPakNode* DPakList::Find(char const* fileName, DPakIndex* index) {
    DPakNode* node = (DPakNode*)patchList_.GetHead();
    while (node) {
        if (node->Find(fileName, index)) return node;
        node = (DPakNode*)node->GetNext();
    }
    node = (DPakNode*)GetHead();
    while (node) {
        if (node->Find(fileName, index)) {
            node->Remove();
            AddHead(node);
            return node;
        }
        node = (DPakNode*)node->GetNext();
    }
    return nullptr;
}

void DPakList::Free() {
    DPakNode* node = (DPakNode*)patchList_.RemoveHead();
    while (node) {
        node->Free();
        node->Delete();
        node = (DPakNode*)RemoveHead();
    }
    node = (DPakNode*)RemoveHead();
    while (node) {
        node->Free();
        node->Delete();
        node = (DPakNode*)RemoveHead();
    }
}
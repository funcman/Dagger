#ifndef DAGGER_LIST_H
#define DAGGER_LIST_H

#include "Node.h"

#include "TypeDef.h"

class DAGGER_API DList {
   protected:
    DNode head_;
    DNode tail_;

   public:
    DList();
    ~DList();
    void Init();
    void Free();
    bool IsEmpty();
    DNode* GetHead();
    DNode* GetTail();
    void AddHead(DNode* pNode);
    void AddTail(DNode* pNode);
    DNode* RemoveHead();
    DNode* RemoveTail();
    int GetCount();
};

#endif//DAGGER_LIST_H

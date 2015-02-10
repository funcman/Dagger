#ifndef __Base__dList__
#define __Base__dList__

#include "dNode.h"

class dList {
protected:
    dNode   head_;
    dNode   tail_;
public:
            dList();
            ~dList();
    void    Init();
    void    Free();
    bool    IsEmpty();
    dNode*  GetHead();
    dNode*  GetTail();
    void    AddHead(dNode* node);
    void    AddTail(dNode* node);
    dNode*  RemoveHead();
    dNode*  RemoveTail();
    long    GetCount();
};

#endif//__Base__dList__

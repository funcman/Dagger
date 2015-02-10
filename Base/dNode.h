#ifndef __Base__dNode__
#define __Base__dNode__

class dNode {
private:
    dNode*  next_;
    dNode*  prev_;
    long    this_;
public:
            dNode();
    bool    IsValid();
    void    SetThis();
    dNode*  GetPrev();
    dNode*  GetNext();
    void    SetPrev(dNode* node);
    void    SetNext(dNode* node);
    bool    InsertBefore(dNode* node);
    bool    InsertAfter(dNode* node);
    bool    Remove();
    void    Delete();
};

#endif//__Base__dNode__

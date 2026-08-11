#ifndef DAGGER_STRLIST_H
#define DAGGER_STRLIST_H

#include "List.h"

class DStrNode : public DNode {
public:
    char name[80];
    void SetName(char const* name);
    char* GetName() {
        return name;
    }
};

class DStrList : public DList {
public:
    DStrNode* FindName(char const* name);
    DStrNode* AddHeadName(char const* name);
    DStrNode* AddTailName(char const* name);
};

#endif//DAGGER_STRLIST_H

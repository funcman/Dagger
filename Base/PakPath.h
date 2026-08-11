#ifndef DAGGER_PAKPATH_H
#define DAGGER_PAKPATH_H

#include <stdio.h>

#include "TypeDef.h"

class DAGGER_API DPakPath {
public:
    DPakPath();
    bool Scan(char const* scanPath, char const* fileName);
    void SetFileExt(char const* fileExt);

private:
    char fileExt_[20];
};

#endif//DAGGER_PAKPATH_H
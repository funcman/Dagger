#ifndef DAGGER_PAKPACK_H
#define DAGGER_PAKPACK_H

#include "TypeDef.h"

class DAGGER_API DPakPack {
public:
    bool Pack(char const* inFile, char const* outFile);
    bool UnPack(char const* inFile, char const* outFile);
};

#endif//DAGGER_PAKPACK_H
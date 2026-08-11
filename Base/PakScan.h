#ifndef DAGGER_PAKSCAN_H
#define DAGGER_PAKSCAN_H

#include <stdio.h>

#include "TypeDef.h"

class DPakScan {
public:
    bool Scan(char const* scanPath, char const* fileName);
};

#endif//DAGGER_PAKSCAN_H
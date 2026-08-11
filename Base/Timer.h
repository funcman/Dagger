#ifndef DAGGER_TIMER_H
#define DAGGER_TIMER_H

#include "TypeDef.h"

class DTimer {
public:
    DTimer();
    void Reset();
    DWORD GetInterval() const;
    bool Passed(DWORD ms);

private:
    DWORD time_;
};

#endif//DAGGER_TIMER_H

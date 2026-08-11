#include "Timer.h"

#include <chrono>

namespace {

// Milliseconds since an arbitrary epoch, from a monotonic clock. The DWORD
// wraparound is harmless: GetInterval subtracts in unsigned arithmetic.
DWORD NowMs() {
    using namespace std::chrono;
    static steady_clock::time_point const epoch = steady_clock::now();
    return (DWORD)duration_cast<milliseconds>(steady_clock::now() - epoch).count();
}

}

DTimer::DTimer() : time_(0) {}

void DTimer::Reset() {
    time_ = NowMs();
}

DWORD DTimer::GetInterval() const {
    return NowMs() - time_;
}

bool DTimer::Passed(DWORD ms) {
    if (GetInterval() >= ms) {
        Reset();
        return true;
    }
    return false;
}

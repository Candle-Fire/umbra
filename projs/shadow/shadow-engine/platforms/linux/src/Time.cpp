#include <shadow/core/Time.h>

size_t SH::Timer::getTimestamp() {
    timespec tick;
    clock_gettime(CLOCK_REALTIME, &tick);
    return size_t(tick.tv_sec * 1000000000 + size_t(tick.tv_nsec));
}
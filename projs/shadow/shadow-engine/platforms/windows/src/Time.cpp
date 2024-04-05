#include <shadow/core/Time.h>

#define INITGUID
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

size_t SH::Timer::getTimestamp() {
    LARGE_INTEGER tick;
    QueryPerformanceCounter(&tick);
    return tick.QuadPart;
}
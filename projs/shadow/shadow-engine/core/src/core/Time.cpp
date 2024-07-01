#include "shadow/core/Time.h"
#include <chrono>

API int SH::Timer::NOW = 0;//SDL_GetPerformanceCounter();
API int SH::Timer::LAST = 0;
API double lastFrame = 0;
API double SH::Timer::deltaTime_ms = 0;
API double SH::Timer::deltaTime = 0;
API double SH::Timer::startTime = 0;
API double SH::Timer::timeSinceStart = 0;

void SH::Timer::UpdateTime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_ms = time_point_cast<milliseconds>(now);

    auto value = now_ms.time_since_epoch();
    double duration = value.count();

    deltaTime = duration - lastFrame;
    if (startTime == 0)
        startTime = duration;
    timeSinceStart = duration - startTime;

    lastFrame = duration;
}

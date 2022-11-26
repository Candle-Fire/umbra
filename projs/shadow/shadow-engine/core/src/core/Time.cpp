#include "core/Time.h"
#include <chrono>

API int Time::NOW = 0;//SDL_GetPerformanceCounter();
API int Time::LAST = 0;
API double lastFrame = 0;
API double Time::deltaTime_ms = 0;
API double Time::deltaTime = 0;
API double Time::startTime = 0;
API double Time::timeSinceStart = 0;

void Time::UpdateTime()
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

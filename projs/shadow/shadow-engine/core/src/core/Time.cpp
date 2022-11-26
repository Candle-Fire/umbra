#include "core/Time.h"
//#include <SDL_hints.h>
//#include <SDL.h>

int Time::NOW = 0;//SDL_GetPerformanceCounter();
int Time::LAST = 0;
double Time::deltaTime_ms = 0;
double Time::deltaTime = 0;

void Time::UpdateTime()
{
    /*
	NOW = SDL_GetTicks();
	deltaTime_ms = LAST > 0 ? (NOW - LAST) *10 : (1.0f / 60.0f);
	deltaTime_ms = deltaTime_ms == 0 ? (1.0f / 60.0f) : deltaTime_ms;

	LAST = NOW;
	deltaTime = deltaTime_ms * 0.001;
    */
}

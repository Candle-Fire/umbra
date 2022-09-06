#pragma once

#ifdef EXPORTING_SH_ENGINE
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

class Time
{

public:
    static API int NOW;
	static API int LAST;

	static API double deltaTime;
	static API double deltaTime_ms;

    static API double timeSinceStart;
    static API double startTime;

	static void UpdateTime();
};

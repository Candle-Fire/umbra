#pragma once


#include "exports.h"

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

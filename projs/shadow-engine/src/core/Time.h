#pragma once

class Time
{
	static int NOW;

public:
	static int LAST;

	static double deltaTime;
	static double deltaTime_ms;

	static void UpdateTime();
};

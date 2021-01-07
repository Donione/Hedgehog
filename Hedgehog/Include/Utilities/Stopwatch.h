#pragma once

#include <chrono>


class Stopwatch
{
public:
	Stopwatch()
	{
		duration = std::chrono::duration<double, std::milli>(0.0);
	}

	void Start()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	void Stop()
	{
		endTime = std::chrono::high_resolution_clock::now();

		duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTime - startTime);
	}

	const std::chrono::duration<double, std::milli>& GetDuration() const { return duration; }

private:
	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point endTime;

	std::chrono::duration<double, std::milli> duration;
};

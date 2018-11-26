//
// Created by wq on 18-11-25.
//

#ifndef CHRONO_H
#define CHRONO_H

#include <iostream>
#include <chrono>

typedef	std::chrono::high_resolution_clock HRClock;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;

using namespace std::chrono;

class Chrono {

	TimePoint				start;
	milliseconds			duration = 3000ms;

public:
	Chrono()
	{
		start = HRClock::now();
	}

	bool isOver() const
	{
		return (HRClock::now() - start > duration);
	}

	void reset()
	{
		start = HRClock::now();
	}

	void setDuration(milliseconds ms)
	{
		duration = ms;
	}
	

};

#endif //CHRONO_H

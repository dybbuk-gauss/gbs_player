#pragma once

#include "timer.h"
#include <vector>

class SystemClock {
  public:
	static const uint64_t CLOCK_RATE = 4194304;

	SystemClock();

	void add_timer(Timer* timer);

	void clock();

  private:
	// not copyable
	SystemClock(const SystemClock&);
	SystemClock& operator=(const SystemClock&);

	std::vector<Timer*> timers;
};

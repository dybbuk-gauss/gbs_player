#include "systemclock.h"
#include <cassert>

/**
 * @brief Constructor of the SystemClock class
 */
SystemClock::SystemClock() {}

/**
 * @brief Add a timer to the system clock
 *
 * @param timer The timer to add
 */
void SystemClock::add_timer(Timer* timer) { timers.push_back(timer); }

/**
 * @brief Clock all timers
 */
void SystemClock::clock() {
	for (auto& timer : timers)
		timer->clock();
}

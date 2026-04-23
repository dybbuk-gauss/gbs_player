#include "timer.h"
#include "systemclock.h"

#include <cassert>

/**
 * @brief Construct a new Timer object
 *
 * @param frequency The frequency of the timer
 */
Timer::Timer(uint64_t frequency) : running(true) { set_frequency(frequency); }

/**
 * @brief Set the frequency of the timer
 *
 * @param frequency The frequency to set
 */
void Timer::set_frequency(uint64_t frequency) {
	this->frequency = frequency;
	period = 0;
	if (frequency > 0)
		period = SystemClock::CLOCK_RATE / frequency;
	counter = period;
}

/**
 * @brief Get the frequency of the timer
 *
 * @return uint64_t The frequency
 */
uint64_t Timer::get_frequency() { return frequency; }

/**
 * @brief Add a listener to the timer
 *
 * @param listener The listener to add
 */
void Timer::add_listener(TimerListener* listener) {
	listeners.push_back(listener);
}

/**
 * @brief Clock the timer
 */
void Timer::clock() {
	if (!running)
		return;

	// To prevent integer underflow
	if (counter > 0)
		--counter;
	if (counter <= 0) {
		counter = period;
		for (auto& listener : listeners)
			listener->clock(this);
	}
}

/**
 * @brief Set the running state of the timer
 *
 * @param running Whether to run the timer
 */
void Timer::set_running(bool running) {
	this->running = running;
	if (!running)
		counter = period;
}

/**
 * @brief Check if the timer is in the first half of the period
 *
 * @return true If the timer is in the first half of the period
 * @return false Otherwise
 */
bool Timer::is_first_half() { return counter >= period / 2; }

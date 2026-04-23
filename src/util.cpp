#include "util.h"
#include <cassert>

#include "systemclock.h"

/**
 * @brief Convert a frequency to a period
 *
 * @param freq The frequency
 * @return uint64_t The period
 */
uint64_t util::to_period(uint64_t freq) {
	return SystemClock::CLOCK_RATE / freq;
}

/**
 * @brief Convert a frequency to a true frequency
 *
 * @param freq The frequency
 * @return uint64_t The true frequency
 */
uint64_t util::to_true_freq(uint16_t freq) {
	return SystemClock::CLOCK_RATE / ((2048 - freq) << 5);
}

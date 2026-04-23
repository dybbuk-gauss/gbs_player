#pragma once

class Timer;

/**
 * @brief Interface for timer listeners
 */
class TimerListener {
  public:
	/**
	 * @brief Clock the listener
	 *
	 * @param timer The timer that was clocked
	 */
	virtual void clock(Timer* timer) = 0;
};

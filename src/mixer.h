#pragma once

#include "channel.h"
#include "timer_listener.h"
#include <atomic>
#include <cstdint>
#include <mutex>

/**
 * @class Mixer
 * @brief It models the mixer of the Game Boy.
 */
class Mixer : public TimerListener {
  public:
	// The sample rate of the Game Boy is 4,194,304 Hz, but we resample it to
	// 44,100 Hz for the player

	static const int SAMPLE_RATE = 44100;

	// the buffer must contain at least this amount of samples
	// before it can be considered ready
	const unsigned int buffer_threshold = 4096;

	Mixer();

	void add_channel(Channel* channel);
	bool buffer_ready();

	Timer& get_timer();

	void set_master_volume(uint8_t left, uint8_t right);
	uint8_t get_master_volume_left();
	uint8_t get_master_volume_right();

	// Returns the range [0, buffer_threshold) from buffer and
	// truncates buffer afterwards
	std::vector<int16_t> pop_buffer();

	virtual void clock(Timer* timer);

  private:
	std::vector<Channel*> channels;
	std::vector<int16_t> buffer;
	Timer timer;
	std::atomic<bool> buf_ready;

	// We need this mutex to prevent the player from reading the buffer
	// (via pop_buffer) while the mixer is updating the buffer
	// (via poll_channels)
	std::mutex buf_mutex;

	uint8_t master_volume_left;
	uint8_t master_volume_right;

	void poll_channels();
};

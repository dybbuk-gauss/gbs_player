#include "mixer.h"
#include <cassert>

/**
 * @brief Construct a new Mixer::Mixer object
 */
Mixer::Mixer() : buf_ready(false) {
	timer.add_listener(this);
	timer.set_frequency(SAMPLE_RATE);
}

/**
 * @brief Add a channel to the mixer
 *
 * @param channel The channel to add
 */
void Mixer::add_channel(Channel* channel) { channels.push_back(channel); }

/**
 * @brief Check if the buffer is ready
 *
 * @return true If the buffer is ready
 * @return false Otherwise
 */
bool Mixer::buffer_ready() { return buf_ready; }

/**
 * @brief Get the timer
 *
 * @return Timer& The timer
 */
Timer& Mixer::get_timer() { return timer; }

/**
 * @brief Set the master volume
 *
 * @param left The left master volume
 * @param right The right master volume
 */
void Mixer::set_master_volume(uint8_t left, uint8_t right) {
	assert(left < 8);
	assert(right < 8);
	master_volume_left = left;
	master_volume_right = right;
}

/**
 * @brief Get the left master volume
 *
 * @return uint8_t The left master volume
 */
uint8_t Mixer::get_master_volume_left() { return master_volume_left; }

/**
 * @brief Get the right master volume
 *
 * @return uint8_t The right master volume
 */
uint8_t Mixer::get_master_volume_right() { return master_volume_right; }

/**
 * @brief Pop the buffer
 *
 * @return std::vector<int16_t> The buffer
 */
std::vector<int16_t> Mixer::pop_buffer() {
	std::lock_guard<std::mutex> lock(buf_mutex);

	if (buffer.size() < buffer_threshold)
		return std::vector<int16_t>(); // 불완전한 버퍼는 반환하지 않음

	auto start = buffer.begin();
	auto end = start + buffer_threshold;
	std::vector<int16_t> vec(start, end);
	buffer.erase(start, end);

	// 버퍼를 꺼낸 후 임계값 상태 업데이트
	buf_ready = (buffer.size() >= buffer_threshold);

	return vec;
}

/**
 * @brief Clock the mixer
 *
 * @param timer The timer
 */
void Mixer::clock(Timer* /*timer*/) { poll_channels(); }

/**
 * @brief Poll the channels and mix the samples
 */
void Mixer::poll_channels() {
	int16_t left_sample = 0;
	int16_t right_sample = 0;
	size_t channel_count = channels.size();
	for (auto& channel : channels) {
		uint8_t sample_raw = channel->get_sample();
		assert(sample_raw < 16);
		double channel_volume = channel->get_true_volume();

		// 1. convert sample from range [0, 15] to [-32768, 32767],
		// 2. scale it (equally for all channels),
		// 3. adjust for the channel volume
		int16_t common_sample = (-32768 + 4369 * sample_raw) *
								(1.0 / channel_count) * channel_volume;

		if (channel->is_left_speaker_enabled())
			left_sample += common_sample * (master_volume_left + 1) / 16.0;
		if (channel->is_right_speaker_enabled())
			right_sample += common_sample * (master_volume_right + 1) / 16.0;
	}

	buf_mutex.lock();
	buffer.push_back(left_sample);
	buffer.push_back(right_sample);

	buf_ready = (buffer.size() >= buffer_threshold);
	buf_mutex.unlock();
}

#include "channel.h"
#include <cassert>

/**
 * @brief Construct a new Channel::Channel object
 */
Channel::Channel()
	: channel_enabled(true), length_counter_limit(64),
	  length_counter_enabled(false), volume_limit(16), frame_sequencer_ticks(0),
	  envelope_period(0), envelope_add(false), dac_enabled(true),
	  length_newly_enabled(false), curr_sample(0), envelope_period_counter(0),
	  left_speaker_enabled(false), right_speaker_enabled(false) {
	timer.add_listener(this);
	set_volume(volume_limit - 1);
	set_timer_frequency(0);
	set_length_counter(0);
}

/**
 * @brief Get the sample from the channel
 *
 * @return uint8_t The sample
 */
uint8_t Channel::get_sample() { return curr_sample; }

/**
 * @brief make a Clock the channel?? idk what this does.
 *
 * @param timer The timer to clock
 */
void Channel::clock(Timer* timer) {
	if (!dac_enabled)
		channel_enabled = false;

	if (timer == &(this->timer)) {
		curr_sample = next_phase();
	} else {
		uint64_t freq = timer->get_frequency();
		// If the frequency is 256, we assume it's the frame sequencer.
		// Not the best way to check, may need to be fixed later

		// 내 생각인데 여기서 포켓몬스터 gbs파일에서만 다음곡 재생을 진행했을 때
		// 간헐적으로 소리가 안나는 문제가
		// 이 부분의 설계 미스에서 왔을수도 있겠음. 아마도?
		if (freq == 256) {
			++frame_sequencer_ticks;
			// update length counter
			if (length_counter_enabled && length_counter > 0) {
				--length_counter;
				if (length_counter <= 0)
					channel_enabled = false;
			}
			// the envelope is updated at 64 Hz, so run this every fourth time
			// the frame sequencer clocks (hence the modulo)
			if (frame_sequencer_ticks % 4 == 0) {
				update_envelope();
			}
		}
	}
}

/**
 * @brief Get the timer
 *
 * @return Timer& The timer
 */
Timer& Channel::get_timer() { return timer; }

/**
 * @brief Set the timer frequency
 *
 * @param frequency The frequency
 */
void Channel::set_timer_frequency(uint64_t frequency) {
	timer.set_frequency(frequency);
}

/**
 * @brief Set the volume
 *
 * @param volume The volume
 * @param set_dac_enabled Whether to set the DAC enabled
 */
void Channel::set_volume(uint8_t volume, bool set_dac_enabled) {
	assert(volume < volume_limit);
	this->volume = volume;
	starting_volume = volume;

	if (set_dac_enabled) {
		if (volume == 0 && !envelope_add) {
			dac_enabled = false;
			channel_enabled = false;
		} else {
			dac_enabled = true;
		}
	}
}

/**
 * @brief Get the volume
 *
 * @return uint8_t The volume
 */
uint8_t Channel::get_volume() { return volume; }

/**
 * @brief Get the true volume
 *
 * @return double The true volume
 */
double Channel::get_true_volume() {
	if (!channel_enabled || !dac_enabled)
		return 0.0;
	return volume / 15.0;
}

/**
 * @brief Set the length counter
 *
 * @param length_counter The length counter
 */
void Channel::set_length_counter(uint16_t length_counter) {
	assert(length_counter <= length_counter_limit);
	this->length_counter = length_counter_limit - length_counter;
	/* std::cout << ">> Len set: " << (unsigned int)(this->length_counter) <<
	 * "\n"; */
}

/**
 * @brief Get the length counter
 *
 * @return uint16_t The length counter
 */
uint16_t Channel::get_length_counter() { return length_counter; }

/**
 * @brief Enable the length counter
 *
 * @param enabled Whether to enable the length counter
 */
void Channel::enable_length_counter(bool enabled) {
	bool newly_enabled = (!length_counter_enabled && enabled);
	length_counter_enabled = enabled;

	length_newly_enabled = newly_enabled;

	/*     /1* std::cout << "PRE " << length_counter << "\n"; *1/ */

	// Clock length counter if length counter goes from disabled to enabled
	// and is in first half of frame sequencer period
	if (length_counter > 0 && newly_enabled && frame_sequencer != NULL &&
		frame_sequencer->is_first_half())
		clock(frame_sequencer);

	/* std::cout << "POST " << length_counter << "\n"; */
}

/**
 * @brief Check if the length counter is enabled
 *
 * @return true If the length counter is enabled
 * @return false Otherwise
 */
bool Channel::is_length_counter_enabled() { return length_counter_enabled; }

/**
 * @brief Set the channel enabled
 *
 * @param enabled Whether to enable the channel
 */
void Channel::set_channel_enabled(bool enabled) { channel_enabled = enabled; }

/**
 * @brief Check if the channel is enabled
 *
 * @return true If the channel is enabled
 * @return false Otherwise
 */
bool Channel::is_channel_enabled() { return channel_enabled; }

/**
 * @brief Set the frame sequencer
 *
 * @param frame_sequencer The frame sequencer
 */
void Channel::set_frame_sequencer(Timer* frame_sequencer) {
	this->frame_sequencer = frame_sequencer;
	frame_sequencer->add_listener(this);
}

/**
 * @brief Set the envelope
 *
 * @param period The period
 * @param add Whether to add to the volume
 */
void Channel::set_envelope(uint8_t period, bool add) {
	assert(period < 8);

	envelope_period = period;
	envelope_period_counter = 0; // the envelope itself begins on trigger
	envelope_add = add;

	if (volume == 0 && !envelope_add) {
		dac_enabled = false;
		channel_enabled = false;
	} else {
		dac_enabled = true;
	}
}

/**
 * @brief Get the envelope period
 *
 * @return uint8_t The envelope period
 */
uint8_t Channel::get_envelope_period() { return envelope_period; }

/**
 * @brief Check if the envelope is adding
 *
 * @return true If the envelope is adding
 * @return false Otherwise
 */
bool Channel::is_envelope_add() { return envelope_add; }

/**
 * @brief Enable the speakers
 *
 * @param left Whether to enable the left speaker
 * @param right Whether to enable the right speaker
 */
void Channel::enable_speakers(bool left, bool right) {
	left_speaker_enabled = left;
	right_speaker_enabled = right;
}

/**
 * @brief Check if the left speaker is enabled
 *
 * @return true If the left speaker is enabled
 * @return false Otherwise
 */
bool Channel::is_left_speaker_enabled() { return left_speaker_enabled; }

/**
 * @brief Check if the right speaker is enabled
 *
 * @return true If the right speaker is enabled
 * @return false Otherwise
 */
bool Channel::is_right_speaker_enabled() { return right_speaker_enabled; }

/**
 * @brief Trigger the channel
 */
void Channel::trigger() {
	if (length_counter == 0) {
		set_length_counter(0);
		// Clock if length counter enabled
		if (length_newly_enabled && frame_sequencer != NULL) {
			clock(frame_sequencer);
		}
	}
	envelope_period_counter = envelope_period;
	set_volume(starting_volume, false);
	if (!dac_enabled)
		channel_enabled = false;
}

/**
 * @brief Update the envelope
 */
void Channel::update_envelope() {
	// Check if it would become 0 if we decremented it now, but don't
	// do it as we want value 0 to mean disabled
	if (envelope_period_counter == 1) {
		if (volume > 0 && !envelope_add)
			--volume;
		else if (volume < 15 && envelope_add)
			++volume;
		envelope_period_counter = envelope_period;
	} else if (envelope_period_counter != 0) {
		--envelope_period_counter;
	}
}

#include "gameboy.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

extern std::atomic<bool> g_quit_requested;

/**
 * @brief Construct a new GameBoy::GameBoy object
 *
 * @param gbs_content The GBS content to use for the GameBoy
 */
GameBoy::GameBoy(GBSContent& gbs_content) : apu(clock), cpu(apu) {
	// apu.run_tests();
	apu.reset();
	this->gbs_content = gbs_content;
	curr_song = gbs_content.first_song;
}

/**
 * @brief Run and execute instructions based on the system clock
 */
void GameBoy::run() {
	// 믹서는 스테레오 샘플 1쌍(2개 데이터)을 생성하므로,
	// 실제 시간 단위인 '샘플 프레임'은 데이터 개수의 절반입니다.
	// mixer create 2 of stereo samples. so real sample frames is half of
	// buffer_threshold
	const int total_buffer_samples = apu.get_mixer().buffer_threshold;
	const int sample_frames = total_buffer_samples / 2;

	const double clocks_per_sample =
		static_cast<double>(SystemClock::CLOCK_RATE) / Mixer::SAMPLE_RATE;
	const uint64_t clocks_per_period =
		static_cast<uint64_t>(clocks_per_sample * sample_frames);
	const long int period_ns = static_cast<long int>(
		(static_cast<double>(sample_frames) / Mixer::SAMPLE_RATE) * 1000000000);
	std::chrono::nanoseconds period_duration(period_ns);

	cpu.gbs_load(gbs_content.load_addr, gbs_content.code);
	cpu.gbs_init(gbs_content.init_addr, curr_song, gbs_content.stack_pointer,
				 gbs_content.timer_modulo, gbs_content.timer_control);

	bool running = true;
	bool init_done = false;

	bool testing = false;

	uint32_t cycles;
	uint32_t instr_cycles;
	uint32_t interrupt_rate;
	uint32_t interrupt_counter = 0;
	while (running && !g_quit_requested) {
		auto start = std::chrono::steady_clock::now();

		// Check if a new song is to be played
		if (play_next_song || play_prev_song) {
			// must reset init_done to ensure a call of a new INIT procedure
			init_done = false;
			if (play_next_song) {
				curr_song =
					(curr_song >= gbs_content.num_songs) ? 1 : curr_song + 1;
				play_next_song = false;
			}
			if (play_prev_song) {
				curr_song =
					(curr_song <= 1) ? gbs_content.num_songs : curr_song - 1;
				play_prev_song = false;
			}
			std::cout << "Playing song no. "
					  << static_cast<unsigned int>(curr_song) << "\n";

			if (player != NULL) {
				player->clear_queue();
			}

			cpu.gbs_init(gbs_content.init_addr, curr_song,
						 gbs_content.stack_pointer, gbs_content.timer_modulo,
						 gbs_content.timer_control);
		}

		interrupt_rate = cpu.get_interrupt_rate();

		cycles = 0;
		while (cycles < clocks_per_period) {
			if (!cpu.is_halted() && !cpu.is_stopped() &&
				(testing || !cpu.procedure_done())) {
				instr_cycles = cpu.execute_instruction();
				cycles += instr_cycles;
				interrupt_counter += instr_cycles;

				for (uint8_t i = 0; i < instr_cycles; ++i)
					clock.clock();
			} else {
				// We still need to run the clock, even if the CPU is
				// halted/stopped or the current sound procedure is done
				clock.clock();
				++cycles;
				++interrupt_counter;
			}

			// Run the play procedure at the end of INIT or at interrupt
			if (!testing) {
				if ((cpu.procedure_done() && !init_done) ||
					(init_done && interrupt_counter >= interrupt_rate)) {
					init_done = true;
					interrupt_counter = 0;
					cpu.gbs_play(gbs_content.play_addr);
				}
			}
		}
		if (!testing && player != NULL)
			player->play();

		// Check if we should quit
		// but in the class CPU, function 'is_hanging' will never return
		// true.
		// because there are no any lines making cpu.hanging = true among whole
		// logic.
		if (cpu.is_hanging()) {
			// Check if there are no queued buffers
			if (player == NULL || !player->has_queued_buffers()) {
				std::cout << "Song finished. Quitting...\n";
				running = false;
			}
		}

		std::this_thread::sleep_until(start + period_duration);
	}
}

/**
 * @brief Get the mixer
 *
 * @return Mixer& The mixer
 */
Mixer& GameBoy::get_mixer() { return apu.get_mixer(); }

/**
 * @brief Set the player
 *
 * @param player The player
 */
void GameBoy::set_player(Player* player) { this->player = player; }

/**
 * @brief Set the next song to play
 */
void GameBoy::next_song() { play_next_song = true; }

/**
 * @brief Set the previous song to play
 */
void GameBoy::prev_song() { play_prev_song = true; }

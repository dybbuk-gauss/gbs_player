#pragma once

#include <condition_variable>

#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <windows.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "mixer.h"

class Player {
  public:
	Player(Mixer& mixer);
	~Player();

	void init();
	void play();
	void clear_queue();		   // 큐 비우기 및 오디오 장치 리셋
	bool has_queued_buffers(); // 큐에 재생 대기 중인 버퍼가 있는지 확인
	size_t get_queue_size();   // 현재 큐에 쌓인 버퍼 개수 확인

  private:
	void audio_loop();			   // 오디오 스레드 메인 루프
	void clean_finished_buffers(); // 재생 완료된 WAVEHDR 정리

	Mixer& mixer;
	HWAVEOUT hWaveOut; // 윈도우 오디오 장치 핸들

	std::thread audio_thread;
	std::mutex mtx;
	std::condition_variable cv;
	std::queue<std::vector<int16_t>> buffer_queue;

	// 재생 중인 버퍼들을 관리하는 큐 (WaveOut 비동기 처리용)
	struct WaveBuffer {
		WAVEHDR header;
		std::vector<int16_t> data;
	};
	std::queue<WaveBuffer*> active_buffers;

	bool running;
};

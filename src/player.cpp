#include "player.h"
#include <cstring>
#include <thread>

Player::Player(Mixer &mixer) : mixer(mixer), hWaveOut(nullptr), running(false) {}

Player::~Player() {
  {
    std::lock_guard<std::mutex> lock(mtx);
    running = false;
  }
  cv.notify_all();
  if (audio_thread.joinable()) {
    audio_thread.join();
  }
  
  if (hWaveOut) {
    waveOutReset(hWaveOut);
    clean_finished_buffers();
    waveOutClose(hWaveOut);
    hWaveOut = nullptr;
  }
}

void Player::init() {
  WAVEFORMATEX wfx;
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = 2;
  wfx.nSamplesPerSec = Mixer::SAMPLE_RATE;
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
  wfx.cbSize = 0;

  if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
    throw std::runtime_error("WaveOut: error opening device");
  }

  running = true;
  audio_thread = std::thread(&Player::audio_loop, this);
}

void Player::play() {
  if (mixer.buffer_ready()) {
    std::vector<int16_t> b = mixer.pop_buffer();
    {
      std::lock_guard<std::mutex> lock(mtx);
      buffer_queue.push(std::move(b));
    }
    cv.notify_one();
  }
}

void Player::clear_queue() {
  std::lock_guard<std::mutex> lock(mtx);
  while (!buffer_queue.empty()) {
    buffer_queue.pop();
  }
  if (hWaveOut) {
    waveOutReset(hWaveOut);
    clean_finished_buffers();
  }
}

void Player::audio_loop() {
  while (true) {
    std::vector<int16_t> current_buffer;
    {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [this] { return !running || !buffer_queue.empty(); });

      if (!running && buffer_queue.empty()) {
        break;
      }

      if (!buffer_queue.empty()) {
        current_buffer = std::move(buffer_queue.front());
        buffer_queue.pop();
      }
    }

    clean_finished_buffers();

    if (!current_buffer.empty() && hWaveOut) {
      WaveBuffer* wb = new WaveBuffer();
      wb->data = std::move(current_buffer);
      
      memset(&wb->header, 0, sizeof(WAVEHDR));
      wb->header.lpData = (LPSTR)wb->data.data();
      wb->header.dwBufferLength = (DWORD)(wb->data.size() * sizeof(int16_t));
      
      waveOutPrepareHeader(hWaveOut, &wb->header, sizeof(WAVEHDR));
      waveOutWrite(hWaveOut, &wb->header, sizeof(WAVEHDR));
      
      active_buffers.push(wb);
    }
  }
}

void Player::clean_finished_buffers() {
  while (!active_buffers.empty()) {
    WaveBuffer* wb = active_buffers.front();
    if (wb->header.dwFlags & WHDR_DONE) {
      waveOutUnprepareHeader(hWaveOut, &wb->header, sizeof(WAVEHDR));
      delete wb;
      active_buffers.pop();
    } else {
      break;
    }
  }
}

bool Player::has_queued_buffers() {
  std::lock_guard<std::mutex> lock(mtx);
  return !buffer_queue.empty();
}

size_t Player::get_queue_size() {
  std::lock_guard<std::mutex> lock(mtx);
  return buffer_queue.size();
}

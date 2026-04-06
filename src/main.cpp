#include "gameboy.h"
#include "gbs_reader.h"
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
// do not change order
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

// 전역 변수로 관리 (인풋 스레드 제어용)
std::atomic<bool> g_running(true);
std::atomic<bool> g_quit_requested(false);

int main(int argc, char *argv[]) {
#ifdef _WIN32
  // Windows의 시스템 타이머 해상도를 1ms로 정밀하게 설정
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  timeBeginPeriod(1);
#endif

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <gbs-file> [track-number]\n";
    return 1;
  }

  // Load the specified file
  GBSReader reader;
  try {
    reader.load_file(argv[1]);
  } catch (std::exception &e) {
    std::cout << "Error while loading file: " << e.what() << "\n";
    std::cout << "Quitting...\n";
    return 1;
  }

  // Print the file's metadata
  reader.print_metadata(true);

  GBSContent content = reader.get_content();

  // Determine the track to play
  uint8_t track = content.first_song;
  if (argc >= 3) {
    try {
      int t = std::stoi(argv[2]);
      if (t >= 1 && t <= content.num_songs) {
        track = static_cast<uint8_t>(t);
      } else {
        std::cout << "Invalid track number. Playing default track "
                  << (int)track << ".\n";
      }
    } catch (...) {
      std::cout << "Invalid track argument. Playing default track "
                << (int)track << ".\n";
    }
  }

  std::cout << "Playing track " << (int)track << " of "
            << (int)content.num_songs << "...\n";
  std::cout << "Press Ctrl+C to stop.\n";

  // Set the track in the content manually since GameBoy constructor uses
  // first_song
  content.first_song = track;

  GameBoy game_boy(content);
  Player player(game_boy.get_mixer());

  // Initialize the sound player
  try {
    player.init();
  } catch (std::exception &e) {
    std::cout << e.what() << "\n";
    return 1;
  };

  game_boy.set_player(&player);

  // 실시간 키 입력 처리를 위한 스레드 (Windows 전용)
#ifdef _WIN32
  std::thread input_thread([&game_boy]() {
    while (g_running) {
      if (_kbhit()) {
        int ch = _getch();
        if (ch == 'n' || ch == 'N') {
          game_boy.next_song();
        } else if (ch == 'p' || ch == 'P') {
          game_boy.prev_song();
        } else if (ch == 'q' || ch == 'Q') {
          g_quit_requested = true;
          break;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  });
#endif

  // Run the game boy logic (this will block until stopped)
  // GameBoy 루프 내부에서 g_quit_requested나 g_running을 체크하도록 수정 필요
  game_boy.run();

  g_running = false;
#ifdef _WIN32
  if (input_thread.joinable()) {
    input_thread.join();
  }
#endif

#ifdef _WIN32
  // 타이머 해상도 설정을 원래대로 복구
  timeEndPeriod(1);
#endif

  return 0;
}

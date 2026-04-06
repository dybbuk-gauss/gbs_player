cmake_minimum_required(VERSION 3.10)
project(gbs_player)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 소스 파일 및 헤더 파일 탐색 (Makefile의 와일드카드 반영)
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# 실행 파일 및 라이브러리 출력 경로를 프로젝트 루트의 'bin' 디렉토리로 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 실행 파일 생성
add_executable(player ${SOURCES} ${HEADERS})

# 컴파일러 옵션 (Makefile의 -Wall -Wextra 반영 및 MSVC 대응)
if(MSVC)
    target_compile_options(player PRIVATE /W4 /MP)
else()
    target_compile_options(player PRIVATE -Wall -Wextra)
endif()

# 라이브러리 의존성 해결 (Threads)
find_package(Threads REQUIRED)
target_link_libraries(player PRIVATE Threads::Threads)

# Windows 전용 고정밀 타이머 및 오디오 라이브러리 추가
if(WIN32)
    target_link_libraries(player PRIVATE winmm)
endif()

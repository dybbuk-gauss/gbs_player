cmake_minimum_required(VERSION 3.10)
project(gbs_player)

# C++ Standard configuration
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory settings - 강제 적용을 위해 전역 설정 유지
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Threads dependency
find_package(Threads REQUIRED)

# Testing setup
enable_testing()
include(CTest)

# Add subdirectories
add_subdirectory(src)

# GTest를 찾을 수 있는 경우에만 tests 추가
# add target tests if  only cmake can find Gtest
find_package(GTest CONFIG QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
else()
    message(WARNING "GTest not found. Tests will not be built.")
endif()

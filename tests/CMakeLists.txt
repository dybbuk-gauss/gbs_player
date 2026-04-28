find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# Create test executable
add_executable(apu_tests apu_test.cpp)

# Link to gbs_core and GTest
target_link_libraries(apu_tests PRIVATE 
    gbs_core 
    GTest::gtest 
    GTest::gtest_main 
    Threads::Threads
)

# 헤더 경로 명시 (gbs_core가 PUBLIC으로 제공하지만 혹시 모르니 추가)
target_include_directories(apu_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Platform specific libraries for tests
if(WIN32)
    target_link_libraries(apu_tests PRIVATE winmm)
endif()

# Discover tests - WORKING_DIRECTORY 설정을 명시
gtest_discover_tests(apu_tests
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

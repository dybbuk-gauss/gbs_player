# --- Core Library ---
# Define all core logic files (excluding main.cpp)
set(CORE_SOURCES
    apu.cpp
    channel.cpp
    cpu.cpp
    gameboy.cpp
    gbs_reader.cpp
    mixer.cpp
    noise.cpp
    opcodes.cpp
    player.cpp
    square1.cpp
    square2.cpp
    systemclock.cpp
    timer.cpp
    util.cpp
    wave.cpp
)

set(CORE_HEADERS
    apu.h
    channel.h
    cpu.h
    gameboy.h
    gbs_reader.h
    mixer.h
    noise.h
    player.h
    square1.h
    square2.h
    systemclock.h
    timer.h
    timer_listener.h
    util.h
    wave.h
)

# Create a static library for the core logic
add_library(gbs_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Include directory for anyone linking to gbs_core
target_include_directories(gbs_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# --- Main Executable ---
add_executable(gbs_player main.cpp)
target_link_libraries(gbs_player PRIVATE gbs_core)

# Platform specific libraries for main app
if(WIN32)
    target_link_libraries(gbs_player PRIVATE winmm)
endif()

# --- Documentation (Doxygen) ---
option(BUILD_DOCS "Build documentation using Doxygen" ON)

if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_EXTRACT_ALL NO)
        set(DOXYGEN_GENERATE_MAN NO)
        set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
        set(DOXYGEN_WARN_IF_INCOMPLETE_DOC NO)


        # Graphviz config
        find_program(DOXYGEN_DOT_EXECUTABLE NAMES dot)
        if(DOXYGEN_DOT_EXECUTABLE)
            set(DOXYGEN_HAVE_DOT YES)
            set(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE})
        else()
            set(DOXYGEN_HAVE_DOT NO)
        endif()

        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")

        doxygen_add_docs(docs
            COMMENT "Generating documentation with Doxygen..."
        )
    endif()
endif()

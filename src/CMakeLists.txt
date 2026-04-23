# --- Documentation ---
# Build documentation only if BUILD_DOCS is ON (default: OFF)
option(BUILD_DOCS "Build documentation using Doxygen" OFF)

if(BUILD_DOCS)
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        set(DOXYGEN_EXTRACT_ALL NO)
        set(DOXYGEN_GENERATE_MAN NO)
        set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
        set(DOXYGEN_WARN_IF_INCOMPLETE_DOC NO)

        # Graphviz 사용 활성화
        # You need set path of dot.exe below.
        set(DOXYGEN_DOT_EXECUTABLE "C:/Program Files/Graphviz/bin/dot.exe")
        find_program(DOXYGEN_DOT_EXECUTABLE NAMES dot)

        if(DOXYGEN_DOT_EXECUTABLE)
            set(DOXYGEN_HAVE_DOT YES)
            set(DOXYGEN_DOT_PATH ${DOXYGEN_DOT_EXECUTABLE})
            message(STATUS "Graphviz found: ${DOXYGEN_DOT_EXECUTABLE}")
        else()
            set(DOXYGEN_HAVE_DOT NO)
            message(WARNING "Graphviz (dot) not found! Diagrams will not be generated.")
        endif()

        # 출력 디렉토리를 루트의 docs 폴더로 설정
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/docs")

        doxygen_add_docs(docs
            "${CMAKE_SOURCE_DIR}/src"
            COMMENT "Making class diagram with Graphviz..."
        )
    endif()
endif()

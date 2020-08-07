cmake_minimum_required(VERSION 3.14)

project(QMidi)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(BUILD_SHARED_LIBS ON)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:\\Qt\\5.15.0\\msvc2019_64\\")

find_package(Qt5 COMPONENTS Core Gui Widgets)

# Common QMidi source files
set(SOURCES
"${CMAKE_SOURCE_DIR}/src/QMidiOut.cpp"
"${CMAKE_SOURCE_DIR}/src/QMidiFile.cpp"
"${CMAKE_SOURCE_DIR}/src/QMidiIn.cpp"
)

# Platform specific QMidi source files
if(WIN32)
    set(SOURCES ${SOURCES}
        "${CMAKE_SOURCE_DIR}/src/OS/QMidi_Win32.cpp"
    )
elseif(UNIX)
    set(SOURCES ${SOURCES}
        "${CMAKE_SOURCE_DIR}/src/OS/QMidi_ALSA.cpp"
    )
elseif(HAIKU)
    set(SOURCES ${SOURCES}
    "${CMAKE_SOURCE_DIR}/src/OS/QMidi_Haiku.cpp"
    )
elseif(APPLE)
    set(SOURCES ${SOURCES}
        "${CMAKE_SOURCE_DIR}/src/OS/QMidi_CoreMidi.cpp"
    )
endif()

# Make sure all source files are present to avoid obscure compiler errors.
foreach(FILE ${SOURCES})
    if(NOT EXISTS "${FILE}")
        message(FATAL_ERROR "File ${FILE} does not exist.")
    endif()
endforeach()

# Add library and specify where to look for headers & libraries
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

# Platform specific MIDI libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm)
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} asound)
elseif(HAIKU)
    target_link_libraries(${PROJECT_NAME} )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework CoreMIDI" "-framework CoreFoundation" "-framework CoreAudio")
endif()
cmake_minimum_required(VERSION 3.14)

project(QMidi)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core)

# Common QMidi source files & library
aux_source_directory("${PROJECT_SOURCE_DIR}/src" SOURCES)
set(LIBRARIES Qt5::Core)

# Platform specific QMidi source files & libraries
if(WIN32)
    set(LIBRARIES ${LIBRARIES} winmm)
    set(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/src/OS/QMidi_Win32.cpp")
elseif(APPLE)
    set(LIBRARIES ${LIBRARIES} "-framework CoreMIDI" "-framework CoreFoundation" "-framework CoreAudio")
    set(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/src/OS/QMidi_CoreMidi.cpp")
elseif(UNIX)
    set(LIBRARIES ${LIBRARIES} asound)
    set(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/src/OS/QMidi_ALSA.cpp")
elseif(HAIKU)
    set(LIBRARIES ${LIBRARIES} midi2)
    set(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/src/OS/QMidi_Haiku.cpp")
endif()

# Add library
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src" ${Qt5Core_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

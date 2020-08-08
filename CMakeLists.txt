cmake_minimum_required(VERSION 3.14)

project(QMidi)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:\\Qt\\5.15.0\\msvc2019_64\\")

find_package(Qt5 COMPONENTS Core)

# Common QMidi source files
set(SOURCES
"${PROJECT_SOURCE_DIR}/src/QMidiOut.cpp"
"${PROJECT_SOURCE_DIR}/src/QMidiFile.cpp"
"${PROJECT_SOURCE_DIR}/src/QMidiIn.cpp"
)

# Platform specific QMidi source files
if(WIN32)
    set(SOURCES ${SOURCES}
        "${PROJECT_SOURCE_DIR}/src/OS/QMidi_Win32.cpp"
    )
elseif(UNIX)
    set(SOURCES ${SOURCES}
        "${PROJECT_SOURCE_DIR}/src/OS/QMidi_ALSA.cpp"
    )
elseif(HAIKU)
    set(SOURCES ${SOURCES}
    "${PROJECT_SOURCE_DIR}/src/OS/QMidi_Haiku.cpp"
    )
elseif(APPLE)
    set(SOURCES ${SOURCES}
        "${PROJECT_SOURCE_DIR}/src/OS/QMidi_CoreMidi.cpp"
    )
endif()

# Make sure all source files are present to avoid obscure compiler errors.
foreach(FILE ${SOURCES})
    if(NOT EXISTS "${FILE}")
        message(FATAL_ERROR "File ${FILE} does not exist.")
    endif()
endforeach()

# Add library and specify where to look for headers & libraries
add_library(${PROJECT_NAME} OBJECT ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/src"
    ${Qt5Core_INCLUDE_DIRS}
)

set(LIBRARIES
    Qt5::Core
)

# Platform specific MIDI libraries
if(WIN32)
    set(LIBRARIES ${LIBRARIES} winmm)
elseif(UNIX)
    set(LIBRARIES ${LIBRARIES} asound)
elseif(HAIKU)
    set(LIBRARIES ${LIBRARIES} midi2)
elseif(APPLE)
    set(LIBRARIES ${LIBRARIES} "-framework CoreMIDI" "-framework CoreFoundation" "-framework CoreAudio")
endif()

# Link static and shared version
add_library(${PROJECT_NAME}_shared SHARED $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}_shared ${LIBRARIES})

add_library(${PROJECT_NAME}_static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}>)
target_link_libraries(${PROJECT_NAME}_static ${LIBRARIES})

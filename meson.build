project('QMidi', 'cpp')

cppCompiler = meson.get_compiler('cpp')

qt5 = import('qt5' )
Qt5_dep = dependency('qt5', method: 'qmake', cmake_module_path: 'C:\\Qt\\5.15.0\\msvc2019_64', modules: ['Core'])

include_dir = include_directories('src/')

# Common QMidi source files & library
sources = ['src/QMidiFile.cpp', 'src/QMidiIn.cpp', 'src/QMidiOut.cpp', qt5.preprocess(moc_headers: ['src/QMidiIn.h'], include_directories: include_dir, dependencies: Qt5_dep)]
dependencies = [Qt5_dep]

# Platform specific QMidi source files & libraries
if build_machine.system() == 'windows'
  dependencies = [dependencies, cppCompiler.find_library('winmm')]
  sources = [sources, 'src/OS/QMidi_Win32.cpp']
elif build_machine.system() == 'linux'
  dependencies = [dependencies, cppCompiler.find_library('asound')]
  sources = [sources, 'src/OS/QMidi_ALSA.cpp', qt5.preprocess(moc_headers: ['src/OS/QMidi_ALSA.h'], include_directories: include_dir, dependencies: Qt5_dep)]
elif build_machine.system() == 'haiku'
  dependencies = [dependencies, cppCompiler.find_library('midi2')]
  sources = [sources, 'src/OS/QMidi_Haiku.cpp']
elif build_machine.system() == 'darwin'
  dependencies = [dependencies, cppCompiler.find_library('-framework CoreMIDI'), cppCompiler.find_library('-framework CoreFoundation'), cppCompiler.find_library('-framework CoreAudio')]
  sources = [sources, 'src/OS/QMidi_CoreMidi.cpp']
endif

# Add library
QMidi_lib = library(meson.project_name(), sources, dependencies: dependencies, include_directories: include_dir)

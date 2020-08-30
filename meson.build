project('QMidi', 'cpp')

cppCompiler = meson.get_compiler('cpp')

qt5 = import('qt5' )
Qt5_dep = dependency('qt5', method: 'qmake', cmake_module_path: 'C:\\Qt\\5.15.0\\msvc2019_64', modules: ['Core'])

include_dir = include_directories('src/')

common_sources = [
  'src/QMidiFile.cpp',
  'src/QMidiIn.cpp',
  'src/QMidiOut.cpp'
]
platform_source = meson.get_external_property('platform_source_suffix')
moc_headers = [
  'src/QMidiIn.h',
  meson.get_external_property('platform_moc_headers')
]
moc_sources = qt5.preprocess(moc_headers: moc_headers, include_directories: include_dir, dependencies: Qt5_dep)

dependencies = [Qt5_dep]
foreach platform_lib: meson.get_external_property('platform_libs')
  dependencies = [dependencies, cppCompiler.find_library(platform_lib)]
endforeach

# Add library
QMidi_lib = library(meson.project_name(), common_sources, platform_source, moc_sources, dependencies: dependencies, include_directories: include_dir)

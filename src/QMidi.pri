# QMidi include file for QMake
CONFIG += c++11

INCLUDEPATH += $$PWD
SOURCES += $$PWD/QMidiOut.cpp \
	$$PWD/QMidiFile.cpp \
	$$PWD/QMidiIn.cpp

HEADERS += $$PWD/QMidiOut.h \
	$$PWD/QMidiFile.h \
	$$PWD/QMidiIn.h

win32 {
	LIBS += -lwinmm
	SOURCES += $$PWD/OS/QMidi_Win32.cpp
}

linux* {
	LIBS += -lasound
	SOURCES += $$PWD/OS/QMidi_ALSA.cpp
	HEADERS += $$PWD/OS/QMidi_ALSA.h
}

haiku* {
	LIBS += -lmidi2
	SOURCES += $$PWD/OS/QMidi_Haiku.cpp
	HEADERS += $$PWD/OS/QMidi_Haiku.h
}

macx* {
	LIBS += -framework CoreMIDI -framework CoreFoundation -framework CoreAudio
	SOURCES += $$PWD/OS/QMidi_CoreMidi.cpp
}

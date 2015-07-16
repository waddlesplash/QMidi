# QMidi include file for QMake
CONFIG += c++11

INCLUDEPATH += $$PWD
SOURCES += $$PWD/QMidiOut.cpp \
	$$PWD/QMidiFile.cpp

HEADERS += $$PWD/QMidiOut.h \
	$$PWD/QMidiFile.h

win32 {
	LIBS += -lwinmm
	SOURCES += $$PWD/OS/QMidi_Win32.cpp
}

linux* {
	LIBS += -lasound
	SOURCES += $$PWD/OS/QMidi_ALSA.cpp
}

haiku* {
	LIBS += -lmidi2
	SOURCES += $$PWD/OS/QMidi_Haiku.cpp
}

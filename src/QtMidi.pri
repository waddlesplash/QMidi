win32 { LIBS += -lwinmm }
linux-g++ { LIBS += -lasound }

SOURCES += $$PWD/QtMidi.cpp \
	$$PWD/QtMidiFile.cpp
HEADERS += $$PWD/QtMidi.h \
	$$PWD/QtMidiFile.h

INCLUDEPATH += $$PWD

win32 { LIBS += -lwinmm }
linux-g++ { LIBS += -lasound }
haiku-g++ { LIBS += -lbe -lmidi -lmidi2 }

SOURCES += $$PWD/QtMidi.cpp \
	$$PWD/QtMidiFile.cpp
HEADERS += $$PWD/QtMidi.h \
	$$PWD/QtMidiFile.h

INCLUDEPATH += $$PWD

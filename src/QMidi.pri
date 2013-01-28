win32 { LIBS += -lwinmm }
linux-g++ { LIBS += -lasound }
haiku-g++ { LIBS += -lbe -lmidi -lmidi2 }

INCLUDEPATH += $$PWD

SOURCES += $$PWD/QMidi.cpp \
	$$PWD/QMidiFile.cpp

HEADERS += $$PWD/QMidi.h \
	$$PWD/QMidiFile.h


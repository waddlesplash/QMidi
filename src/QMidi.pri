win32 { LIBS += -lwinmm }
linux-g++ { LIBS += -lasound }
haiku-g++ { LIBS += -lbe -lmidi -lmidi2 }

INCLUDEPATH += $$PWD

SOURCES += $$PWD/QMidiOut.cpp \
	$$PWD/QMidiFile.cpp

HEADERS += $$PWD/QMidiOut.h \
	$$PWD/QMidiFile.h


win32 { LIBS += -lwinmm }
linux* { LIBS += -lasound }
haiku* { LIBS += -lbe -lmidi -lmidi2 }

INCLUDEPATH += $$PWD

SOURCES += $$PWD/QMidiOut.cpp \
	$$PWD/QMidiFile.cpp

HEADERS += $$PWD/QMidiOut.h \
	$$PWD/QMidiFile.h


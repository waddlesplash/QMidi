# QtPlaySMF QMakefile

QT = core
CONFIG += console
TEMPLATE = app
TARGET = qtplaysmf

include(../../src/QMidi.pri)

SOURCES += playsmf.cpp

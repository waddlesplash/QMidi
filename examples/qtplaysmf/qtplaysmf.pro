
QT += core
QT -= gui
CONFIG += console
TEMPLATE = app
TARGET = qtplaysmf

include(../../src/QtMidi.pri)

SOURCES += playsmf.cpp

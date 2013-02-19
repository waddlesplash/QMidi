/*
 * qmidi (QMidi.h)
 *  Part of QMidi (http://github.com/waddlesplash/qtmidi).
 *
 * Copyright (c) 2012 WaddleSplash
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
*/

#ifndef QMIDI_H
#define QMIDI_H

#include <QObject>
#include <QMap>
#include <QString>

class QMidi : public QObject
{
    Q_OBJECT
public:
    static QMap<QString, QString> outDeviceNames();
    static bool initMidiOut(QString outDeviceId);
    static void closeMidiOut();
    static void outSendMsg(qint32 msg);

    static void outSetInstr(int voice, int instr);
    static void outNoteOn(int note, int voice, int velocity = 64);
    static void outNoteOff(int note, int voice);
    static void outPitchWheel(int voice, int value);
    static void outStopAll();
    static void outStopAll(int voice);

signals:
    void allNotesStopped(int voice);

private:
    static QString myOutDeviceId;
};

#endif // QMIDI_H

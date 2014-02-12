/*
 * QMidiOut (QMidiOut.h)
 *  Part of QMidi (http://github.com/waddlesplash/qtmidi).
 *
 * Copyright (c) 2012-2014 WaddleSplash
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

#ifndef QMIDIOUT_H
#define QMIDIOUT_H

#include <QObject>
#include <QMap>
#include <QString>

class QMidiEvent;
struct MidiPtrObjs;

class QMidiOut : public QObject
{
    Q_OBJECT
public:
    static QMap<QString, QString> devices();

    QMidiOut(QObject* parent = 0);
    bool connect(QString outDeviceId);
    void disconnect();
    void sendMsg(qint32 msg);
    void sendEvent(QMidiEvent* e);

    void setInstr(int voice, int instr);
    void noteOn(int note, int voice, int velocity = 64);
    void noteOff(int note, int voice);
    void pitchWheel(int voice, int value);
    void controlChange(int voice, int number, int value);
    void stopAll();
    void stopAll(int voice);

signals:
    void allNotesStopped(int voice);

private:
    QString myDeviceId;
    MidiPtrObjs* myMidiPtrs;
    bool myConnected;
};

#endif // QMIDIOUT_H

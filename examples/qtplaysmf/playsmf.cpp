/*
 * QtPlaySMF
 *  Part of QtMidi (http://github.com/waddlesplash/qtmidi).
 *
 * Copyright (c) 2003-2012 by David G. Slomin
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

#include <stdio.h>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QMidiOut.h>
#include <QMidiFile.h>

class MidiPlayer : public QThread
{
    Q_OBJECT
public:
    MidiPlayer(QMidiFile* file)
    { midi_file = file; }

private:
    QMidiEvent* midi_file_event;
    QMidiFile* midi_file;
protected:
    void run()
    {
        QElapsedTimer t;
        t.start();
        QList<QMidiEvent*>* events = midi_file->events();
        for(int i = 0; i < events->count(); i++)
        {
            QMidiEvent* midi_file_event = events->at(i);
            if (midi_file_event->type() != QMidiEvent::Meta)
            {
                qint64 event_time = midi_file->timeFromTick(midi_file_event->tick()) * 1000;

                qint32 waitTime = event_time - t.elapsed();
                if(waitTime > 0) {
                    msleep(waitTime);
                }
                handleEvent();
            }
        }

        QMidiOut::closeMidiOut();
    }
private slots:
    void handleEvent()
    {
        if (midi_file_event->type() == QMidiEvent::SysEx)
        { // TODO: sysex
        }
        else
        {
            qint32 message = midi_file_event->message();
            QMidiOut::outSendMsg(message);
        }
    }
};

static void usage(char *program_name)
{
    fprintf(stderr, "Usage: %s -p<port> <MidiFile>\n\n", program_name);
    fprintf(stderr, "Ports:\nID	Name\n----------------\n");
    QMap<QString,QString> vals = QMidiOut::outDeviceNames();
    foreach(QString key,vals.keys())
    {
        QString value = vals.value(key);
        fprintf(stderr,key.toUtf8().constData());
        fprintf(stderr,"	");
        fprintf(stderr,value.toUtf8().constData());
        fprintf(stderr,"\n");
    }
    exit(1);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc,argv);

    QString filename = "";
    QString midiOutName = "";
    QMidiFile* midi_file = new QMidiFile();

    for(int i = 1; i < argc; i++)
    {
        QString curArg(argv[i]);
        if((curArg == "--help") || (curArg == "-h") || (curArg == "/?") || (curArg == "/help"))
        { usage(argv[0]); }
        else if(curArg.startsWith("-p"))
        {
            midiOutName = curArg.mid(2);
        }
        else if(filename == "")
        { filename = argv[i]; }
        else
        { usage(argv[0]); }
    }

    if((filename == "") || (midiOutName == ""))
    {
        usage(argv[0]);
    }
    midi_file->load(filename);

    QMidiOut::initMidiOut(midiOutName);

    MidiPlayer* p = new MidiPlayer(midi_file);
    QObject::connect(p,SIGNAL(finished()),&a,SLOT(quit()));
    p->start();

    return a.exec();
}
#include "playsmf.moc"

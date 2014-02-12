/*
 * QMidiOut (QMidiOut.cpp)
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

#include "QMidiOut.h"

#include <QStringList>
#include "QMidiFile.h"

#if defined(Q_OS_WIN)
    #include <windows.h>
    #include <mmsystem.h>
    struct MidiPtrObjs {
        HMIDIOUT midiOutPtr;
    };
#elif defined(Q_OS_LINUX)
    #include <alsa/asoundlib.h>
    #include <alsa/seq.h>
    #include <alsa/seq_midi_event.h>
    struct MidiPtrObjs {
        snd_seq_t *midiOutPtr;
    };
#elif defined(Q_OS_HAIKU)
    #include <MidiRoster.h>
    #include <MidiConsumer.h>
    #include <MidiProducer.h>
    struct MidiPtrObjs {
        BMidiConsumer* midiOutConsumer;
        BMidiLocalProducer* midiOutLocProd;
    };
#endif

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
    QMap<QString, QString> ret;

#if defined(Q_OS_WIN)
    int numDevs = midiOutGetNumDevs();
    if(numDevs == 0) {
        return ret;
    }

    for(int i = 0; i < numDevs; i++) {
        MIDIOUTCAPS* devCaps = new MIDIOUTCAPS;
        midiOutGetDevCaps(i, devCaps, sizeof(*devCaps));
        ret.insert(QString::number(i), QString::fromWCharArray(devCaps->szPname));
        delete devCaps;
    }
#elif defined(Q_OS_LINUX)
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;
    int client;
    int err;
    snd_seq_t *handle;

    err = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if(err < 0) {
        /* Use snd_strerror(errno) to get the error here. */
        return ret;
    }

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_client_info_set_client(cinfo, -1);

    while(snd_seq_query_next_client(handle, cinfo) >= 0) {
        client = snd_seq_client_info_get_client(cinfo);
        snd_seq_port_info_alloca(&pinfo);
        snd_seq_port_info_set_client(pinfo, client);

        snd_seq_port_info_set_port(pinfo, -1);
        while(snd_seq_query_next_port(handle, pinfo) >= 0) {
            int cap = (SND_SEQ_PORT_CAP_SUBS_WRITE|SND_SEQ_PORT_CAP_WRITE);
            if((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
                QString port = QString::number(snd_seq_port_info_get_client(pinfo));
                port += ":" + QString::number(snd_seq_port_info_get_port(pinfo));
                QString name = snd_seq_client_info_get_name(cinfo);
                ret.insert(port, name);
            }
        }
    }
#elif defined(Q_OS_HAIKU)
    bool OK = true;
    int32 id = 0;
    while(OK) {
        BMidiConsumer* c = BMidiRoster::NextConsumer(&id);
        if(c != NULL) {
            ret.insert(QString::number(id), QString::fromUtf8(c->Name()));
            c->Release();
        } else {
            OK = false;
        }
    }
#endif

    return ret;
}


QMidiOut::QMidiOut(QObject *parent)
    : QObject(parent)
{
    myMidiPtrs = new MidiPtrObjs;
    myConnected = false;
}

bool QMidiOut::connect(QString outDeviceId)
{
    disconnect();

#if defined(Q_OS_WIN)
    midiOutOpen(&myMidiPtrs->midiOutPtr, outDeviceId.toInt(), 0, 0, CALLBACK_NULL);
#elif defined(Q_OS_LINUX)
    int err = snd_seq_open(&myMidiPtrs->midiOutPtr, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if(err < 0) {
        return false;
    }
    snd_seq_set_client_name(myMidiPtrs->midiOutPtr, "QtMidi");

    snd_seq_create_simple_port(myMidiPtrs->midiOutPtr, "Output Port",
                               SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

    QStringList l = outDeviceId.split(":");
    int client = l.at(0).toInt();
    int port = l.at(1).toInt();
    snd_seq_connect_to(myMidiPtrs->midiOutPtr, 0, client, port);
#elif defined(Q_OS_HAIKU)
    myMidiPtrs->midiOutConsumer = BMidiRoster::FindConsumer(outDeviceId.toInt());
    if(myMidiPtrs->midiOutConsumer == NULL) {
        return false;
    }
    myMidiPtrs->midiOutLocProd = new BMidiLocalProducer("QMidi");
    if(!myMidiPtrs->midiOutLocProd->IsValid()) {
        myMidiPtrs->midiOutLocProd->Release();
        return false;
    }
    myMidiPtrs->midiOutLocProd->Register();
    if(myMidiPtrs->midiOutLocProd->Connect(myMidiPtrs->midiOutConsumer) != B_OK) {
        return false;
    }
#endif
    myDeviceId = outDeviceId;
    myConnected = true;
    return true;
}

void QMidiOut::disconnect()
{
    if(!myConnected) { return; }

#if defined(Q_OS_WIN)
    midiOutClose(myMidiPtrs->midiOutPtr);
#elif defined(Q_OS_LINUX)
    QStringList l = myDeviceId.split(":");
    int client = l.at(0).toInt();
    int port = l.at(1).toInt();

    snd_seq_disconnect_from(myMidiPtrs->midiOutPtr, 0, client,port);
#elif defined(Q_OS_HAIKU)
    myMidiPtrs->midiOutLocProd->Disconnect(myMidiPtrs->midiOutConsumer);
    myMidiPtrs->midiOutConsumer->Release();
    myMidiPtrs->midiOutLocProd->Unregister();
    myMidiPtrs->midiOutLocProd->Release();
#endif

    myConnected = false;
}

void QMidiOut::sendMsg(qint32 msg)
{
    if(!myConnected) { return; }

#if !defined(Q_OS_WIN)
    char buf[3];
    buf[0] = msg & 0xFF;
    buf[1] = (msg >> 8) & 0xFF;
    buf[2] = (msg >> 16) & 0xFF;
#endif
  
#if defined(Q_OS_WIN)
    midiOutShortMsg(myMidiPtrs->midiOutPtr, (DWORD)msg);
#elif defined(Q_OS_LINUX)
    snd_seq_event_t ev;
    snd_midi_event_t* mev;

    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, 0);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    snd_midi_event_new(3, &mev);
    snd_midi_event_resize_buffer(mev, 3);
    snd_midi_event_encode(mev,(unsigned char*)&buf, 3, &ev);

    snd_seq_event_output(myMidiPtrs->midiOutPtr, &ev);
    snd_seq_drain_output(myMidiPtrs->midiOutPtr);
#elif defined(Q_OS_HAIKU)
    myMidiPtrs->midiOutLocProd->SprayData((void*)&buf, 3, true);
#endif
}

void QMidiOut::sendEvent(QMidiEvent* e)
{
    sendMsg(e->message());
}

void QMidiOut::setInstr(int voice, int instr)
{
    qint32 msg = 0x0000C0 + voice;
    msg |= instr<<8;
    sendMsg(msg);
}

void QMidiOut::noteOn(int note, int voice, int velocity)
{
    qint32 msg = 0x90 + voice;
    msg |= note<<8;
    msg |= velocity<<16;
    sendMsg(msg);
}

void QMidiOut::noteOff(int note, int voice)
{
    qint32 msg = 0x80 + voice;
    msg |= note<<8;
    sendMsg(msg);
}

void QMidiOut::pitchWheel(int voice, int value)
{
    qint32 msg = 0xE0 + voice;
    msg |= (value & 0x7F)<<8; // fine adjustment (ignored by many synths)
    msg |= (value / 128)<<16; // coarse adjustment
    sendMsg(msg);
}

void QMidiOut::controlChange(int voice, int number, int value)
{
    qint32 msg = 0xB0 + voice;
    msg |= (number)<<8;
    msg |= (value)<<16;
    sendMsg(msg);
}

void QMidiOut::stopAll()
{
    for(int i = 0; i < 16; i++) {
        stopAll(i);
    }
}

void QMidiOut::stopAll(int voice)
{
    sendMsg((0xB0 | voice) | (0x7B<<8));
}

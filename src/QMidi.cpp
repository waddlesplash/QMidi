/*
 * qmidi (QMidi.cpp)
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

#include "QMidi.h"

#if defined(Q_OS_WIN)
#   include <windows.h> // MmSystem needs DWORD, etc.
#   include <mmsystem.h> // For MIDI In/Out on Win
HMIDIOUT midiOutPtr;
#elif defined(Q_OS_LINUX)
#   include <alsa/asoundlib.h>
#   include <alsa/seq.h>
#   include <alsa/seq_midi_event.h>
#   include <QStringList>
snd_seq_t *midiOutPtr;
#elif defined(Q_OS_HAIKU)
#	include <MidiConsumer.h>
#	include <MidiProducer.h>
#	include <MidiRoster.h>
BMidiConsumer* midiOutConsumer;
BMidiLocalProducer* midiOutLocProd;
#endif

// TODO: error reporting

QString QMidi::myOutDeviceId;

QMap<QString,QString> QMidi::outDeviceNames()
{
    QMap<QString,QString> ret;

#if defined(Q_OS_WIN)
    int numDevs = midiOutGetNumDevs();
    if(numDevs == 0) { return ret; }

    for(int i = 0;i<numDevs;i++) {
        MIDIOUTCAPS* devCaps = new MIDIOUTCAPS;
        midiOutGetDevCaps(i,devCaps,sizeof(*devCaps));
        ret.insert(QString::number(i),QString::fromWCharArray(devCaps->szPname));
        delete devCaps;
    }
#elif defined(Q_OS_LINUX)
    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;
    int client;
    int err;
    snd_seq_t *handle;

    err = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if(err < 0)
    { /* Could not open sequencer!! use  snd_strerror(errno)  to get error. */ return ret; }

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
                ret.insert(port,name);
            }
        }
    }
#elif defined(Q_OS_HAIKU)
	bool OK = true;
	int32 id = 0;
	while(OK) {
		BMidiConsumer* c = BMidiRoster::NextConsumer(&id);
		if(c != NULL) {
			ret.insert(QString::number(id),QString::fromUtf8(c->Name()));
			c->Release();
		} else {
			OK = false;
		}
	}
#endif

    return ret;
}

bool QMidi::initMidiOut(QString outDeviceId)
{
#if defined(Q_OS_WIN)
    midiOutOpen(&midiOutPtr,outDeviceId.toInt(),0,0,CALLBACK_NULL);
#elif defined(Q_OS_LINUX)
    int err = snd_seq_open(&midiOutPtr, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if(err < 0) { return false; }
    snd_seq_set_client_name(midiOutPtr, "QtMidi");

    snd_seq_create_simple_port(midiOutPtr, "Output Port",
                               SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);

    QStringList l = outDeviceId.split(":");
    int client = l.at(0).toInt();
    int port = l.at(1).toInt();
    snd_seq_connect_to(midiOutPtr, 0, client, port);
#elif defined(Q_OS_HAIKU)
	midiOutConsumer = BMidiRoster::FindConsumer(outDeviceId.toInt());
	if(midiOutConsumer == NULL) { return false; }
	midiOutLocProd = new BMidiLocalProducer("QtMidi");
	if(!midiOutLocProd->IsValid()) { midiOutLocProd->Release(); return false; } // some error ??
	midiOutLocProd->Register();
	if(midiOutLocProd->Connect(midiOutConsumer) != B_OK) { return false; }
#endif
    myOutDeviceId = outDeviceId;
    return true;
}

void QMidi::closeMidiOut()
{
#if defined(Q_OS_WIN)
    midiOutClose(midiOutPtr);
#elif defined(Q_OS_LINUX)
    QStringList l = myOutDeviceId.split(":");
    int client = l.at(0).toInt();
    int port = l.at(1).toInt();

    snd_seq_disconnect_from(midiOutPtr, 0, client,port);
#elif defined(Q_OS_HAIKU)
	midiOutLocProd->Disconnect(midiOutConsumer);
	midiOutConsumer->Release();
	midiOutLocProd->Unregister();
	midiOutLocProd->Release();
#endif
}

void QMidi::outSendMsg(qint32 msg)
{
#if defined(Q_OS_WIN)
    midiOutShortMsg(midiOutPtr,(DWORD)msg);
#elif defined(Q_OS_LINUX)
    snd_seq_event_t ev;
    snd_midi_event_t* mev;

    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, 0);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    snd_midi_event_new(sizeof(msg), &mev);
    snd_midi_event_resize_buffer(mev, sizeof(msg));
    snd_midi_event_encode(mev,(unsigned char*)&msg, sizeof(msg), &ev);

    snd_seq_event_output(midiOutPtr, &ev);
    snd_seq_drain_output(midiOutPtr);
#elif defined(Q_OS_HAIKU)
    midiOutLocProd->SprayData((void*)&msg,sizeof(msg),true);
#endif
}

void QMidi::outSetInstr(int voice, int instr)
{
    qint32 msg = 0x0000C0 + voice;
    msg |= instr<<8;
    outSendMsg(msg);
}

void QMidi::outNoteOn(int note, int voice, int velocity)
{
    qint32 msg = 0x90 + voice;
    msg |= note<<8;
    msg |= velocity<<16;
    outSendMsg(msg);
}

void QMidi::outNoteOff(int note, int voice)
{
    qint32 msg = 0x80 + voice;
    msg |= note<<8;
    outSendMsg(msg);
}

void QMidi::outPitchWheel(int voice, int value)
{
    qint32 msg = 0xE0 + voice;
    msg |= (value & 0x7F)<<8; // fine adjustment
    msg |= (value / 128)<<16; // coarse adjustment
    outSendMsg(msg);
}

void QMidi::outStopAll()
{
    for(int i = 0;i<16;i++)
    { outStopAll(i); }
}

void QMidi::outStopAll(int voice)
{
    outSendMsg((0xB0 | voice) | (0x7B<<8));
}

/*
 * qmidi (QMidiFile.cpp)
 *  Part of QMidi (http://github.com/waddlesplash/qtmidi).
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

#include "QMidiFile.h"

#include <QFile>
#include <stdlib.h>

QMidiEvent::QMidiEvent()
{
    myTrackNumber = -1;
    myType = Invalid;
    myVoice = -1;
    myNote = -1;
    myVelocity = -1;
    myAmount = -1;
    myNumber = -1;
    myValue = -1;
    myNumerator = -1;
    myDenominator = -1;
    myData = "";
    myTick = -1;
}

quint32 QMidiEvent::message()
{
    union
    {
        unsigned char data_as_bytes[4];
        quint32 data_as_uint32;
    } u;

    switch(myType)
    {
    case NoteOff:
        u.data_as_bytes[0] = 0x80 | myVoice;
        u.data_as_bytes[1] = myNote;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case NoteOn:
        u.data_as_bytes[0] = 0x90 | myVoice;
        u.data_as_bytes[1] = myNote;
        u.data_as_bytes[2] = myVelocity;
        u.data_as_bytes[3] = 0;
        break;

    case KeyPressure:
        u.data_as_bytes[0] = 0xA0 | myVoice;
        u.data_as_bytes[1] = myNote;
        u.data_as_bytes[2] = myAmount;
        u.data_as_bytes[3] = 0;
        break;

    case ControlChange:
        u.data_as_bytes[0] = 0xB0 | myVoice;
        u.data_as_bytes[1] = myNumber;
        u.data_as_bytes[2] = myValue;
        u.data_as_bytes[3] = 0;
        break;

    case ProgramChange:
        u.data_as_bytes[0] = 0xC0 | myVoice;
        u.data_as_bytes[1] = myNumber;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case ChannelPressure:
        u.data_as_bytes[0] = 0xD0 | myVoice;
        u.data_as_bytes[1] = myAmount;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case PitchWheel:
        u.data_as_bytes[0] = 0xE0 | myVoice;
        u.data_as_bytes[2] = myValue >> 7;
        u.data_as_bytes[1] = myValue;
        u.data_as_bytes[3] = 0;
        break;

    default:
        return 0;
        break;
    }
    return u.data_as_uint32;
}

void QMidiEvent::setMessage(quint32 data)
{
    union
    {
        quint32 data_as_uint32;
        unsigned char data_as_bytes[4];
    } u;

    u.data_as_uint32 = data;

    switch (u.data_as_bytes[0] & 0xF0)
    {
    case 0x80:
    {
        setType(NoteOff);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setNote(u.data_as_bytes[1]);
        setVelocity(u.data_as_bytes[2]);
        return;
    }
    case 0x90:
    {
        setType(NoteOn);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setNote(u.data_as_bytes[1]);
        setVelocity(u.data_as_bytes[2]);
        return;
    }
    case 0xA0:
    {
        setType(KeyPressure);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setNote(u.data_as_bytes[1]);
        setAmount(u.data_as_bytes[2]);
        return;
    }
    case 0xB0:
    {
        setType(ControlChange);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setNumber(u.data_as_bytes[1]);
        setValue(u.data_as_bytes[2]);
        return;
    }
    case 0xC0:
    {
        setType(ProgramChange);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setNumber(u.data_as_bytes[1]);
        return;
    }
    case 0xD0:
    {
        setType(ChannelPressure);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setAmount(u.data_as_bytes[1]);
        return;
    }
    case 0xE0:
    {
        setType(PitchWheel);
        setVoice(u.data_as_bytes[0] & 0x0F);
        setValue((u.data_as_bytes[2] << 7) | u.data_as_bytes[1]);
        return;
    }
    }
}

float QMidiEvent::tempo()
{
    unsigned char* buffer;
    qint32 midi_tempo = 0;

    if (!this->type() == Meta_Tempo) { return -1; }

    buffer = (unsigned char*)myData.constData();
    midi_tempo = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
    return (float)(60000000.0 / midi_tempo);
}

/* End of QtMidiEvent functions, on to QtMidiFile */

QMidiFile::QMidiFile()
{
    myDivType = PPQ;
    myFileFormat = 0;
    myResolution = 0;
}

bool isGreaterThan(QMidiEvent* e1,QMidiEvent* e2)
{
    qint32 e1t = e1->tick();
    qint32 e2t = e2->tick();
    if(e1t < e2t) { return true; }
    else { return false; }
}

void QMidiFile::sort()
{
    qStableSort(myEvents.begin(),myEvents.end(),isGreaterThan);
    qStableSort(myTempoEvents.begin(),myTempoEvents.end(),isGreaterThan);
}

void QMidiFile::addEvent(qint32 tick, QMidiEvent* e)
{
    e->setTick(tick);
    myEvents.append(e);
    if(!disableSort) { sort(); }
    if((e->track() == 0) && (e->type() == QMidiEvent::Meta_Tempo))
    {
        myTempoEvents.append(e);
    }
}
void QMidiFile::removeEvent(QMidiEvent* e)
{
    myEvents.removeOne(e);
    if((e->track() == 0) && (e->type() == QMidiEvent::Meta_Tempo))
    { myTempoEvents.removeOne(e); }
}

QList<QMidiEvent*> QMidiFile::eventsForTrack(int track)
{
    QList<QMidiEvent*> ret;
    QMidiEvent* e;
    foreach(e,myEvents)
    {
        if(e->track() == track)
        { ret.append(e); }
    }
    return ret;
}

QList<QMidiEvent*> QMidiFile::events(int voice)
{
    QList<QMidiEvent*> ret;
    QMidiEvent* e;
    foreach(e,myEvents)
    {
        if(e->voice() == voice)
        { ret.append(e); }
    }
    return ret;
}

int QMidiFile::createTrack()
{
    int t = myTracks.count();
    myTracks.append(t);
    return t;
}

void QMidiFile::removeTrack(int track)
{
    if(myTracks.contains(track))
    { myTracks.removeOne(track); }
}

QMidiEvent* QMidiFile::createNoteOffEvent(int track, qint32 tick, int voice, int note, int velocity)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::NoteOff);
    e->setTrack(track);
    e->setVoice(voice);
    e->setNote(note);
    e->setVelocity(velocity);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createNoteOnEvent(int track, qint32 tick, int voice, int note, int velocity)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::NoteOn);
    e->setTrack(track);
    e->setVoice(voice);
    e->setNote(note);
    e->setVelocity(velocity);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createKeyPressureEvent(int track, qint32 tick, int voice, int note, int amount)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::KeyPressure);
    e->setTrack(track);
    e->setVoice(voice);
    e->setNote(note);
    e->setAmount(amount);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createChannelPressureEvent(int track, qint32 tick, int voice, int amount)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::ChannelPressure);
    e->setTrack(track);
    e->setVoice(voice);
    e->setAmount(amount);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createControlChangeEvent(int track, qint32 tick, int voice, int number, int value)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::ControlChange);
    e->setTrack(track);
    e->setVoice(voice);
    e->setNumber(number);
    e->setValue(value);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createProgramChangeEvent(int track, qint32 tick, int voice, int number)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::ProgramChange);
    e->setTrack(track);
    e->setVoice(voice);
    e->setNumber(number);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createPitchWheelEvent(int track, qint32 tick, int voice, int value)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::PitchWheel);
    e->setTrack(track);
    e->setVoice(voice);
    e->setValue(value);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createSysexEvent(int track, qint32 tick, QByteArray data)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::SysEx);
    e->setTrack(track);
    e->setData(data);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createMetaEvent(int track, qint32 tick, int number, QByteArray data)
{
    QMidiEvent* e = new QMidiEvent();
    switch(number)
    {
    case 0x51:
        e->setType(QMidiEvent::Meta_Tempo);
        break;
    case 0x58:
        e->setType(QMidiEvent::Meta_TimeSignature);
        break;
    case 0x5:
        e->setType(QMidiEvent::Meta_Lyric);
        break;
    case 0x6:
        e->setType(QMidiEvent::Meta_Marker);
        break;
    default:
        e->setType(QMidiEvent::Meta);
        break;
    }
    e->setTrack(track);
    e->setNumber(number);
    e->setData(data);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createTempoEvent(int track, qint32 tick, float tempo)
{
    long midi_tempo = 60000000L / tempo;
    QByteArray buffer;
    buffer[0] = (midi_tempo >> 16) & 0xFF;
    buffer[1] = (midi_tempo >> 8) & 0xFF;
    buffer[2] = midi_tempo & 0xFF;
    return createMetaEvent(track, tick, 0x51, buffer);
}
QMidiEvent* QMidiFile::createTimeSignatureEvent(int track, qint32 tick, int numerator, int denominator)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::Meta_TimeSignature);
    e->setTrack(track);
    e->setNumerator(numerator);
    e->setDenominator(denominator);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createLyricEvent(int track, qint32 tick, QByteArray text)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::Meta_Lyric);
    e->setTrack(track);
    e->setData(text);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createMarkerEvent(int track, qint32 tick, QByteArray text)
{
    QMidiEvent* e = new QMidiEvent();
    e->setType(QMidiEvent::Meta_Marker);
    e->setTrack(track);
    e->setData(text);
    addEvent(tick,e);
    return e;
}
QMidiEvent* QMidiFile::createVoiceEvent(int track, qint32 tick, quint32 data)
{
    QMidiEvent* e = new QMidiEvent();
    e->setTrack(track);
    e->setMessage(data);
    addEvent(tick,e);
    return e;
}

float QMidiFile::timeFromTick(qint32 tick)
{
    switch(myDivType)
    {
    case PPQ:
    {
        float tempo_event_time = 0.0;
        qint32 tempo_event_tick = 0;
        float tempo = 120.0;

        foreach(QMidiEvent* e,myTempoEvents)
        {
            if(e->tick() >= tick) { break; }
            tempo_event_time += (((float)(e->tick() - tempo_event_tick)) / myResolution / (tempo / 60));
            tempo_event_tick = e->tick();
            tempo = e->tempo();
        }

        float time = tempo_event_time + (((float)(tick - tempo_event_tick)) / myResolution / (tempo / 60));
        return time;
    }
    case SMPTE24:
    {
        return (float)(tick) / (myResolution * 24.0);
    }
    case SMPTE25:
    {
        return (float)(tick) / (myResolution * 25.0);
    }
    case SMPTE30DROP:
    {
        return (float)(tick) / (myResolution * 29.97);
    }
    case SMPTE30:
    {
        return (float)(tick) / (myResolution * 30.0);
    }
    default:
    {
        return -1;
    }
    }
}

qint32 QMidiFile::tickFromTime(float time)
{
    switch(myDivType)
    {
    case PPQ:
    {
        float tempo_event_time = 0.0;
        qint32 tempo_event_tick = 0;
        float tempo = 120.0;

        foreach(QMidiEvent* e,myTempoEvents)
        {
            float next_tempo_event_time = tempo_event_time + (((float)(e->tick() - tempo_event_tick)) / myResolution / (tempo / 60));
            if (next_tempo_event_time >= time) break;
            tempo_event_time = next_tempo_event_time;
            tempo_event_tick = e->tick();
            tempo = e->tempo();
        }

        return tempo_event_tick + (qint32)((time - tempo_event_time) * (tempo / 60) * myResolution);
    }
    case SMPTE24:
    {
        return (qint32)(time * myResolution * 24.0);
    }
    case SMPTE25:
    {
        return (qint32)(time * myResolution * 25.0);
    }
    case SMPTE30DROP:
    {
        return (qint32)(time * myResolution * 29.97);
    }
    case SMPTE30:
    {
        return (qint32)(time * myResolution * 30.0);
    }
    default:
    {
        return -1;
    }
    }
}

float QMidiFile::beatFromTick(qint32 tick)
{
    switch(myDivType)
    {
    case PPQ:
    {
        return (float)(tick) / myResolution;
    }
    case SMPTE24:
    {
        return -1.0; /* TODO */
    }
    case SMPTE25:
    {
        return -1.0; /* TODO */
    }
    case SMPTE30DROP:
    {
        return -1.0; /* TODO */
    }
    case SMPTE30:
    {
        return -1.0; /* TODO */
    }
    default:
    {
        return -1.0;
    }
    }
}

qint32 QMidiFile::tickFromBeat(float beat)
{
    switch(myDivType)
    {
    case PPQ:
    {
        return (qint32)(beat * myResolution);
    }
    case SMPTE24:
    {
        return -1; /* TODO */
    }
    case SMPTE25:
    {
        return -1; /* TODO */
    }
    case SMPTE30DROP:
    {
        return -1; /* TODO */
    }
    case SMPTE30:
    {
        return -1; /* TODO */
    }
    default:
    {
        return -1;
    }
    }
}

/*
 * Helpers
 */

qint16 interpret_int16(unsigned char *buffer)
{
    return ((qint16)(buffer[0]) << 8) | (qint16)(buffer[1]);
}
quint16 interpret_uint16(unsigned char *buffer)
{
    return ((quint16)(buffer[0]) << 8) | (quint16)(buffer[1]);
}
quint16 read_uint16(QFile *in)
{
    unsigned char buffer[2];
    in->read((char*)buffer,2);
    return interpret_uint16(buffer);
}
void write_uint16(QFile *out, quint16 value)
{
    unsigned char buffer[2];
    buffer[0] = (unsigned char)((value >> 8) & 0xFF);
    buffer[1] = (unsigned char)(value & 0xFF);
    out->write((char*)buffer,2);
}

quint32 read_uint32(QFile *in)
{
    unsigned char buffer[4];
    in->read((char*)&buffer,4);
    return ((quint32)(buffer[0]) << 24) | ((quint32)(buffer[1]) << 16) | ((quint32)(buffer[2]) << 8) | (quint32)(buffer[3]);
}
void write_uint32(QFile *out, quint32 value)
{
    unsigned char buffer[4];
    buffer[0] = (unsigned char)(value >> 24);
    buffer[1] = (unsigned char)((value >> 16) & 0xFF);
    buffer[2] = (unsigned char)((value >> 8) & 0xFF);
    buffer[3] = (unsigned char)(value & 0xFF);
    out->write((char*)buffer,4);
}

quint32 read_variable_length_quantity(QFile *in)
{
    unsigned char b;
    quint32 value = 0;

    do
    {
        in->getChar((char*)&b);
        value = (value << 7) | (b & 0x7F);
    }
    while ((b & 0x80) == 0x80);

    return value;
}
void write_variable_length_quantity(QFile *out, quint32 value)
{
    unsigned char buffer[4];
    int offset = 3;

    while(1)
    {
        buffer[offset] = (unsigned char)(value & 0x7F);
        if (offset < 3) buffer[offset] |= 0x80;
        value >>= 7;
        if ((value == 0) || (offset == 0)) { break; }
        offset--;
    }

    out->write((char*)buffer+offset,4-offset);
}

bool QMidiFile::load(QString filename)
{
    QFile in(filename);

    if(!in.exists() || !in.open(QFile::ReadOnly)) { return false; }

    disableSort = true;
    unsigned char chunk_id[4], division_type_and_resolution[4];
    qint32 chunk_size = 0, chunk_start = 0;
    int file_format = 0, number_of_tracks = 0, number_of_tracks_read = 0;

    in.read((char*)chunk_id,4);
    chunk_size = read_uint32(&in);
    chunk_start = in.pos();

    /* check for the RMID variation on SMF */

    if (memcmp(chunk_id, "RIFF", 4) == 0)
    {
        in.read((char*)chunk_id,4);
        /* technically this one is a type id rather than a chunk id */

        if (memcmp(chunk_id, "RMID", 4) != 0)
        {
            in.close();
            disableSort = false;
            return false;
        }

        in.read((char*)chunk_id,4);
        chunk_size = read_uint32(&in);

        if (memcmp(chunk_id, "data", 4) != 0)
        {
            in.close();
            disableSort = false;
            return false;
        }

        in.read((char*)chunk_id,4);
        chunk_size = read_uint32(&in);
        chunk_start = in.pos();
    }

    if (memcmp(chunk_id, "MThd", 4) != 0)
    {
        in.close();
        disableSort = false;
        return false;
    }

    file_format = read_uint16(&in);
    number_of_tracks = read_uint16(&in);
    in.read((char*)division_type_and_resolution,2);

    switch ((signed char)(division_type_and_resolution[0]))
    {
    case -24:
    {
        myFileFormat = file_format;
        myDivType = SMPTE24;
        myResolution = division_type_and_resolution[1];
        break;
    }
    case -25:
    {
        myFileFormat = file_format;
        myDivType = SMPTE25;
        myResolution = division_type_and_resolution[1];
        break;
    }
    case -29:
    {
        myFileFormat = file_format;
        myDivType = SMPTE30DROP;
        myResolution = division_type_and_resolution[1];
        break;
    }
    case -30:
    {
        myFileFormat = file_format;
        myDivType = SMPTE30;
        myResolution = division_type_and_resolution[1];
        break;
    }
    default:
    {
        myFileFormat = file_format;
        myDivType = PPQ;
        myResolution = interpret_uint16(division_type_and_resolution);
        break;
    }
    }

    /* forwards compatibility:  skip over any extra header data */
    in.seek(chunk_start + chunk_size);

    while (number_of_tracks_read < number_of_tracks)
    {
        in.read((char*)chunk_id,4);
        chunk_size = read_uint32(&in);
        chunk_start = in.pos();

        if (memcmp(chunk_id, "MTrk", 4) == 0)
        {
            int track = createTrack();
            qint32 tick = 0, previous_tick = 0;
            unsigned char status, running_status = 0;
            int at_end_of_track = 0;

            while ((in.pos() < chunk_start + chunk_size) && !at_end_of_track)
            {
                tick = read_variable_length_quantity(&in) + previous_tick;
                previous_tick = tick;

                in.getChar((char*)&status);

                if ((status & 0x80) == 0x00)
                {
                    status = running_status;
                    in.seek(in.pos()-1);
                }
                else
                {
                    running_status = status;
                }

                switch (status & 0xF0)
                {
                case 0x80:
                {
                    int channel = status & 0x0F;
                    char note;
                    in.getChar(&note);
                    char velocity;
                    in.getChar(&velocity);
                    createNoteOffEvent(track, tick, channel, note, velocity);
                    break;
                }
                case 0x90:
                {
                    int channel = status & 0x0F;
                    char note;
                    in.getChar(&note);
                    char velocity;
                    in.getChar(&velocity);
                    if(velocity != 0)
                    { createNoteOnEvent(track, tick, channel, note, velocity); }
                    else { createNoteOffEvent(track, tick, channel, note); }
                    break;
                }
                case 0xA0:
                {
                    int channel = status & 0x0F;
                    char note;
                    in.getChar(&note);
                    char amount;
                    in.getChar(&amount);
                    createKeyPressureEvent(track, tick, channel, note, amount);
                    break;
                }
                case 0xB0:
                {
                    int channel = status & 0x0F;
                    char number;
                    in.getChar(&number);
                    char value;
                    in.getChar(&value);
                    createControlChangeEvent(track, tick, channel, number, value);
                    break;
                }
                case 0xC0:
                {
                    int channel = status & 0x0F;
                    char number;
                    in.getChar(&number);
                    createProgramChangeEvent(track, tick, channel, number);
                    break;
                }
                case 0xD0:
                {
                    int channel = status & 0x0F;
                    char amount;
                    in.getChar(&amount);
                    createChannelPressureEvent(track, tick, channel, amount);
                    break;
                }
                case 0xE0:
                {
                    int channel = status & 0x0F;
                    char value;
                    in.getChar(&value);
                    char value2;
                    in.getChar(&value2);

                    qint16 pitch;
                    pitch = ((value2&0x7F) << 7) | (value&0x7F); // Unpack 14-bit value

                    createPitchWheelEvent(track, tick, channel, pitch);
                    break;
                }
                case 0xF0:
                {
                    switch (status)
                    {
                    case 0xF0:
                    case 0xF7:
                    {
                        int data_length = read_variable_length_quantity(&in) + 1;
                        QByteArray data;
                        data[0] = status;
                        data += in.read(data_length-1);

                        createSysexEvent(track, tick, data);
                        break;
                    }
                    case 0xFF:
                    {
                        char number;
                        in.getChar(&number);
                        int data_length = read_variable_length_quantity(&in);
                        QByteArray data = in.read(data_length);

                        if (number == 0x2F) {
                            setTrackEndTick(track, tick);
                            at_end_of_track = 1;
                        } else {
                            createMetaEvent(track, tick, number, data);
                        }
                        break;
                    }
                    }

                    break;
                }
                }
            }

            number_of_tracks_read++;
        }

        /* forwards compatibility:  skip over any unrecognized chunks, or extra data at the end of tracks */
        in.seek(chunk_start + chunk_size);
    }

    in.close();
    disableSort = false;
    sort();
    return true;
}

bool QMidiFile::save(QString filename)
{
    QFile out(filename);
    int curTrack;

    if(out.exists()) { out.remove(); }
    if((filename == "") || !(out.open(QFile::WriteOnly)))
    { return false; }

    out.write("MThd",4);
    write_uint32(&out, 6);
    write_uint16(&out, (quint16)(myFileFormat));
    write_uint16(&out, (quint16)(myTracks.size()));

    switch(myDivType) {
    case PPQ:
        write_uint16(&out, (quint16)(myResolution));
        break;
    default:
        out.putChar(myDivType);
        out.putChar(myResolution);
        break;
    }

    foreach(curTrack,myTracks) {
        QMidiEvent* e;
        qint32 track_size_offset, track_start_offset, track_end_offset, tick, previous_tick;

        out.write("MTrk",4);

        track_size_offset = out.pos();
        write_uint32(&out, 0);

        track_start_offset = out.pos();

        previous_tick = 0;

        QList<QMidiEvent*> eventsForTrk = eventsForTrack(curTrack);
        foreach(e,eventsForTrk) {
            tick = e->tick();
            write_variable_length_quantity(&out, tick - previous_tick);

            switch(e->type()) {
            case QMidiEvent::NoteOff:
                out.putChar(0x80 | (e->voice() & 0x0F));
                out.putChar(e->note() & 0x7F);
                out.putChar(e->velocity() & 0x7F);
                break;

            case QMidiEvent::NoteOn:
                out.putChar(0x90 | (e->voice() & 0x0F));
                out.putChar(e->note() & 0x7F);
                out.putChar(e->velocity() & 0x7F);
                break;

            case QMidiEvent::KeyPressure:
                out.putChar(0xA0 | (e->voice() & 0x0F));
                out.putChar(e->note() & 0x7F);
                out.putChar(e->amount() & 0x7F);
                break;

            case QMidiEvent::ControlChange:
                out.putChar(0xB0 | (e->voice() & 0x0F));
                out.putChar(e->number() & 0x7F);
                out.putChar(e->value() & 0x7F);
                break;

            case QMidiEvent::ProgramChange:
                out.putChar(0xC0 | (e->voice() & 0x0F));
                out.putChar(e->number() & 0x7F);
                break;

            case QMidiEvent::ChannelPressure:
                out.putChar(0xD0 | (e->voice() & 0x0F));
                out.putChar(e->value() & 0x7F);
                break;

            case QMidiEvent::PitchWheel:
            {
                int value = e->value();
                out.putChar(0xE0 | (e->voice() & 0x0F));
                out.putChar(value & 0x7F);
                out.putChar((value >> 7) & 0x7F);
                break;
            }
            case QMidiEvent::SysEx:
            {
                int data_length = e->data().size();
                unsigned char* data = (unsigned char*)e->data().constData();
                out.putChar(data[0]);
                write_variable_length_quantity(&out, data_length - 1);
                out.write((char*)data+1,data_length-1);
                break;
            }
            case QMidiEvent::Meta:
            case QMidiEvent::Meta_Lyric:
            case QMidiEvent::Meta_Tempo:
            case QMidiEvent::Meta_Marker:
            case QMidiEvent::Meta_TimeSignature:
            {
                int data_length = e->data().size();
                unsigned char *data = (unsigned char*)e->data().constData();
                out.putChar(0xFF);
                out.putChar(e->number() & 0x7F);
                write_variable_length_quantity(&out, data_length);
                out.write((char*)data,data_length);
                break;
            }
            default:
                break;
            }

            previous_tick = tick;
        }

        write_variable_length_quantity(&out, trackEndTick(curTrack) - previous_tick);
        out.write("\xFF\x2F\x00",3);

        track_end_offset = out.pos();

        out.seek(track_size_offset);
        write_uint32(&out, track_end_offset - track_start_offset);

        out.seek(track_end_offset);
    }

    out.close();
    return true;
}

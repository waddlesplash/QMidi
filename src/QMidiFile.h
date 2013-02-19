/*
 * qmidi (QMidiFile.h)
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

#ifndef QMIDIFILE_H
#define QMIDIFILE_H

#include <QString>
#include <QMap>
#include <QList>

class QMidiEvent
{
public:
    enum EventType {
        Invalid = -1,
        NoteOn,
        NoteOff,
        KeyPressure,
        ChannelPressure,
        ControlChange,
        ProgramChange,
        PitchWheel,
        Meta = 256, // so we can do Meta | Meta_Tempo...
        Meta_Tempo = 256+1,
        Meta_TimeSignature = 256+2,
        Meta_Lyric = 256+3,
        Meta_Marker = 256+4,
        SysEx
    };
    
    QMidiEvent();
    inline ~QMidiEvent() {}
    
    inline EventType type() { return myType; }
    inline void setType(EventType newType) { myType = newType; }

    inline qint32 tick() { return myTick; }
    inline void setTick(qint32 tick) { myTick = tick; }
    /* you MUST run the QtMidiFile's sort() function after changing ticks! */
    /* otherwise, it will not play or write the file properly! */

    inline int track() { return myTrackNumber; }
    inline void setTrack(int trackNumber) { myTrackNumber = trackNumber; }

    inline int voice() { return myVoice; }
    inline void setVoice(int voice) { myVoice = voice; }

    inline int note() { return myNote; }
    inline void setNote(int note) { myNote = note; }

    inline int velocity() { return myVelocity; }
    inline void setVelocity(int velocity) { myVelocity = velocity; }

    inline int amount() { return myAmount; }
    inline void setAmount(int amount) { myAmount = amount; }

    inline int number() { return myNumber; }
    inline void setNumber(int number) { myNumber = number; }

    inline int value() { return myValue; }
    inline void setValue(int value) { myValue = value; }

    float tempo();

    inline int numerator() { return myNumerator; }
    inline void setNumerator(int numerator) { myNumerator = numerator; }

    inline int denominator() { return myDenominator; }
    inline void setDenominator(int denominator) { myDenominator = denominator; }

    inline QByteArray data() { return myData; }
    inline void setData(QByteArray data) { myData = data; }

    quint32 message();
    void setMessage(quint32 data);

    inline bool isNoteEvent() { return ((myType == NoteOn) || (myType == NoteOff)); }
    
private:
    int myVoice;
    int myNote;
    int myVelocity;
    int myAmount; // KeyPressure, ChannelPressure
    int myNumber; // ControlChange, ProgramChange
    int myValue; // PitchWheel, ControlChange
    int myNumerator; // TimeSignature
    int myDenominator; // TimeSignature
    QByteArray myData; // Meta, SysEx

    qint32 myTick;
    EventType myType;
    int myTrackNumber;
};

class QMidiFile
{
public:
    enum DivisionType {
        /* These types match the MIDI values for them.
         * DON'T CHANGE OR YOU WON'T BE ABLE TO READ/WRITE FILES! */
        Invalid = -1,
        PPQ = 0,
        SMPTE24 = -24,
        SMPTE25 = -25,
        SMPTE30DROP = -29,
        SMPTE30 = -30
    };

    QMidiFile();
    inline ~QMidiFile() {}

    bool load(QString filename);
    bool save(QString filename);

    void sort();

    inline void setFileFormat(int fileFormat) { myFileFormat = fileFormat; }
    inline int fileFormat() { return myFileFormat; }

    inline void setResolution(int resolution) { myResolution = resolution; }
    inline int resolution() { return myResolution; }

    inline void setDivisionType(DivisionType type) { myDivType = type; }
    inline DivisionType divisionType() { return myDivType; }

    void addEvent(qint32 tick, QMidiEvent* e);
    void removeEvent(QMidiEvent* e);

    int createTrack();
    void removeTrack(int track);
    inline void setTrackEndTick(int track, qint32 tick) { myTrackEndTicks.insert(track,tick); }
    inline qint32 trackEndTick(int track) { return myTrackEndTicks.value(track); }
    inline QList<int> tracks() { return myTracks; }

    QMidiEvent* createNoteOffEvent(int track, qint32 tick, int voice, int note, int velocity = 64);
    /* velocity here is how fast to stop the note (127=fastest) */
    QMidiEvent* createNoteOnEvent(int track, qint32 tick, int voice, int note, int velocity);

    inline QMidiEvent* createNote(int track, qint32 start_tick, qint32 end_tick, int voice, int note, int start_velocity, int end_velocity)
    { createNoteOffEvent(track,end_tick,voice,note,end_velocity); return createNoteOnEvent(track,start_tick,voice,note,start_velocity); }
    /* returns the start event */

    QMidiEvent* createKeyPressureEvent(int track, qint32 tick, int voice, int note, int amount);
    QMidiEvent* createChannelPressureEvent(int track, qint32 tick, int voice, int amount);
    QMidiEvent* createControlChangeEvent(int track, qint32 tick, int voice, int number, int value);
    QMidiEvent* createProgramChangeEvent(int track, qint32 tick, int voice, int number);
    QMidiEvent* createPitchWheelEvent(int track, qint32 tick, int voice, int value);
    QMidiEvent* createSysexEvent(int track, qint32 tick, QByteArray data);
    QMidiEvent* createMetaEvent(int track, qint32 tick, int number, QByteArray data);
    QMidiEvent* createTempoEvent(int track, qint32 tick, float tempo); /* tempo is in BPM */
    QMidiEvent* createTimeSignatureEvent(int track, qint32 tick, int numerator, int denominator);
    QMidiEvent* createLyricEvent(int track, qint32 tick, QByteArray text);
    QMidiEvent* createMarkerEvent(int track, qint32 tick, QByteArray text);
    QMidiEvent* createVoiceEvent(int track, qint32 tick, quint32 data);

    inline QList<QMidiEvent*> events() { return myEvents; }
    QList<QMidiEvent*> events(int voice);
    QList<QMidiEvent*> eventsForTrack(int track);

    float timeFromTick(qint32 tick); /* time is in seconds */
    qint32 tickFromTime(float time);
    float beatFromTick(qint32 tick);
    qint32 tickFromBeat(float beat);

private:
    QList<QMidiEvent*> myEvents;
    QList<QMidiEvent*> myTempoEvents;
    QList<int> myTracks;
    QMap<qint16,qint32> myTrackEndTicks;
    DivisionType myDivType;
    qint16 myResolution;
    qint16 myFileFormat;
    bool disableSort;
};


#endif // QTMIDIFILE_H

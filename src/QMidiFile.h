/*
 * Copyright 2003-2012 by David G. Slomin
 * Copyright 2012-2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

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
		Meta,
		SysEx
	};
	enum MetaNumbers {
		/* These types match the MIDI values for them.
		 * DON'T CHANGE OR YOU WON'T BE ABLE TO READ/WRITE FILES! */
		TrackName = 0x03,
		Tempo = 0x51,
		TimeSignature = 0x58,
		Lyric = 0x5,
		Marker = 0x6
	};

	QMidiEvent();
	~QMidiEvent();

	inline EventType type() { return fType; }
	inline void setType(EventType newType) { fType = newType; }

	inline qint32 tick() { return fTick; }
	inline void setTick(qint32 tick) { fTick = tick; }
	/* you MUST run the QMidiFile's sort() function after changing ticks! */
	/* otherwise, it will not play or write the file properly! */

	inline int track() { return fTrackNumber; }
	inline void setTrack(int trackNumber) { fTrackNumber = trackNumber; }

	inline int voice() { return fVoice; }
	inline void setVoice(int voice) { fVoice = voice; }

	inline int note() { return fNote; }
	inline void setNote(int note) { fNote = note; }

	inline int velocity() { return fVelocity; }
	inline void setVelocity(int velocity) { fVelocity = velocity; }

	inline int amount() { return fAmount; }
	inline void setAmount(int amount) { fAmount = amount; }

	inline int number() { return fNumber; }
	inline void setNumber(int number) { fNumber = number; }

	inline int value() { return fValue; }
	inline void setValue(int value) { fValue = value; }

	float tempo();

	inline int numerator() { return fNumerator; }
	inline void setNumerator(int numerator) { fNumerator = numerator; }

	inline int denominator() { return fDenominator; }
	inline void setDenominator(int denominator) { fDenominator = denominator; }

	inline QByteArray data() { return fData; }
	inline void setData(QByteArray data) { fData = data; }

	quint32 message() const;
	void setMessage(quint32 data);

	inline bool isNoteEvent() { return ((fType == NoteOn) || (fType == NoteOff)); }

private:
	int fVoice;
	int fNote;
	int fVelocity;
	int fAmount;	// KeyPressure, ChannelPressure
	int fNumber;	// ControlChange, ProgramChange, Meta
	int fValue;		// PitchWheel, ControlChange
	int fNumerator; // TimeSignature
	int fDenominator; // TimeSignature
	QByteArray fData; // Meta, SysEx

	qint32 fTick;
	EventType fType;
	int fTrackNumber;
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
	~QMidiFile();

	void clear();
	bool load(QString filename);
	bool save(QString filename);

	QMidiFile* oneTrackPerVoice();

	void sort();

	inline void setFileFormat(int fileFormat) { fFileFormat = fileFormat; }
	inline int fileFormat() { return fFileFormat; }

	inline void setResolution(int resolution) { fResolution = resolution; }
	inline int resolution() { return fResolution; }

	inline void setDivisionType(DivisionType type) { fDivType = type; }
	inline DivisionType divisionType() { return fDivType; }

	void addEvent(qint32 tick, QMidiEvent* e);
	void removeEvent(QMidiEvent* e);

	int createTrack();
	void removeTrack(int track);
	qint32 trackEndTick(int track);
	inline QList<int> tracks() { return fTracks; }

	QMidiEvent* createNoteOnEvent(int track, qint32 tick, int voice, int note, int velocity);
	QMidiEvent* createNoteOffEvent(int track, qint32 tick, int voice, int note, int velocity = 64);
	/* velocity on NoteOff events is how fast to stop the note (127=fastest) */

	QMidiEvent* createNote(int track, qint32 start_tick, qint32 end_tick, int voice, int note,
						   int start_velocity, int end_velocity);
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

	inline QList<QMidiEvent*> events() { return QList<QMidiEvent*>(fEvents); }
	QList<QMidiEvent*> events(int voice);
	QList<QMidiEvent*> eventsForTrack(int track);

	float timeFromTick(qint32 tick); /* time is in seconds */
	qint32 tickFromTime(float time);
	float beatFromTick(qint32 tick);
	qint32 tickFromBeat(float beat);

private:
	QList<QMidiEvent*> fEvents;
	QList<QMidiEvent*> fTempoEvents;
	QList<int> fTracks;
	DivisionType fDivType;
	int fResolution;
	int fFileFormat;

	bool fDisableSort;
};

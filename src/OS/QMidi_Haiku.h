/*
 * Copyright 2019 Georg Gadinger <nilsding@nilsding.org>
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <MidiConsumer.h>

class QMidiIn;

namespace QMidiInternal
{
class MidiInConsumer : public BMidiLocalConsumer
{
public:
	MidiInConsumer(QMidiIn* midiIn, const char *name = NULL);

	void ChannelPressure(uchar channel, uchar pressure, bigtime_t time) override;
	void ControlChange(uchar channel, uchar controlNumber, uchar controlValue, bigtime_t time) override;
	void KeyPressure(uchar channel, uchar note, uchar pressure, bigtime_t time) override;
	void NoteOff(uchar channel, uchar note, uchar velocity, bigtime_t time) override;
	void NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time) override;
	void PitchBend(uchar channel, uchar lsb, uchar msb, bigtime_t time) override;
	void ProgramChange(uchar channel, uchar programNumber, bigtime_t time) override;
	void SystemExclusive(void* data, size_t length, bigtime_t time) override;

private:
	QMidiIn* fMidiIn;
};
}

/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include "QMidiFile.h"

// TODO: error reporting

QMidiOut::QMidiOut()
	: fMidiPtrs(NULL),
	  fConnected(false)
{
}
QMidiOut::~QMidiOut()
{
	if (fConnected)
		disconnect();
}

void QMidiOut::sendEvent(const QMidiEvent& e)
{
	if (e.type() == QMidiEvent::SysEx) {
		sendSysEx(e.data());
		return;
	}

	sendMsg(e.message());
}

void QMidiOut::setInstrument(int voice, int instr)
{
	qint32 msg = 0xC0 + voice;
	msg |= instr << 8;
	sendMsg(msg);
}

void QMidiOut::noteOn(int note, int voice, int velocity)
{
	qint32 msg = 0x90 + voice;
	msg |= note << 8;
	msg |= velocity << 16;
	sendMsg(msg);
}

void QMidiOut::noteOff(int note, int voice, int velocity)
{
	qint32 msg = 0x80 + voice;
	msg |= note << 8;
	msg |= velocity << 16;
	sendMsg(msg);
}

void QMidiOut::pitchWheel(int voice, int value)
{
	qint32 msg = 0xE0 + voice;
	msg |= (value & 0x7F) << 8; // fine adjustment (ignored by many synths)
	msg |= (value / 128) << 16; // coarse adjustment
	sendMsg(msg);
}

void QMidiOut::channelAftertouch(int voice, int value)
{
	qint32 msg = 0xD0 + voice;
	msg |= value << 8;
	sendMsg(msg);
}

void QMidiOut::polyphonicAftertouch(int note, int voice, int value)
{
	qint32 msg = 0xA0 + voice;
	msg |= note << 8;
	msg |= value << 16;
	sendMsg(msg);
}

void QMidiOut::controlChange(int voice, int number, int value)
{
	qint32 msg = 0xB0 + voice;
	msg |= number << 8;
	msg |= value << 16;
	sendMsg(msg);
}

void QMidiOut::stopAll()
{
	for (int i = 0; i < 16; i++)
		stopAll(i);
}

void QMidiOut::stopAll(int voice)
{
	sendMsg((0xB0 | voice) | (0x7B << 8));
}

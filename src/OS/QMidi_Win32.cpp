/*
 * Copyright 2012-2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include "QMidiFile.h"

#include <QStringList>

#include <windows.h>
#include <mmsystem.h>
struct MidiPtrObjs {
	HMIDIOUT midiOutPtr;
};

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
	QMap<QString, QString> ret;

	int numDevs = midiOutGetNumDevs();
	if (numDevs == 0) {
		return ret;
	}

	for (int i = 0; i < numDevs; i++) {
		MIDIOUTCAPS* devCaps = new MIDIOUTCAPS;
		midiOutGetDevCaps(i, devCaps, sizeof(*devCaps));
		ret.insert(QString::number(i), QString::fromWCharArray(devCaps->szPname));
		delete devCaps;
	}

	return ret;
}

bool QMidiOut::connect(QString outDeviceId)
{
	if (myConnected)
		disconnect();
	myMidiPtrs = new MidiPtrObjs;

	midiOutOpen(&myMidiPtrs->midiOutPtr, outDeviceId.toInt(), 0, 0, CALLBACK_NULL);

	myDeviceId = outDeviceId;
	myConnected = true;
	return true;
}

void QMidiOut::disconnect()
{
	if (!myConnected) {
		return;
	}

	midiOutClose(myMidiPtrs->midiOutPtr);

	delete myMidiPtrs;
	myMidiPtrs = NULL;
	myConnected = false;
}

void QMidiOut::sendMsg(qint32 msg)
{
	if (!myConnected) {
		return;
	}

	midiOutShortMsg(myMidiPtrs->midiOutPtr, (DWORD)msg);
}

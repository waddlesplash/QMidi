/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include <QStringList>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

struct NativeMidiOutInstances {
	HMIDIOUT midiOut;
};

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
	QMap<QString, QString> ret;

	int numDevs = midiOutGetNumDevs();
	if (numDevs == 0)
		return ret;

	for (int i = 0; i < numDevs; i++) {
		MIDIOUTCAPSW devCaps;
		midiOutGetDevCapsW(i, &devCaps, sizeof(MIDIOUTCAPSW));
		ret.insert(QString::number(i), QString::fromWCharArray(devCaps.szPname));
	}

	return ret;
}

bool QMidiOut::connect(QString outDeviceId)
{
	if (fConnected)
		disconnect();
	fMidiPtrs = new NativeMidiOutInstances;

	midiOutOpen(&fMidiPtrs->midiOut, outDeviceId.toInt(), 0, 0, CALLBACK_NULL);

	fDeviceId = outDeviceId;
	fConnected = true;
	return true;
}

void QMidiOut::disconnect()
{
	if (!fConnected)
		return;

	midiOutClose(fMidiPtrs->midiOut);
	fConnected = false;

	delete fMidiPtrs;
	fMidiPtrs = NULL;
}

void QMidiOut::sendMsg(qint32 msg)
{
	if (!fConnected)
		return;

	midiOutShortMsg(fMidiPtrs->midiOut, (DWORD)msg);
}

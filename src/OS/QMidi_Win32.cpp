/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"
#include "QMidiIn.h"
#include "QMidiFile.h"

#include <QStringList>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

// # pragma mark - QMidiOut

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

void QMidiOut::sendSysEx(const QByteArray &data)
{
	if (!fConnected)
		return;

	MIDIHDR header;
	memset(&header, 0, sizeof(MIDIHDR));

	header.lpData = (LPSTR) data.data();
	header.dwBufferLength = data.length();

	// TODO: check for retval of midiOutPrepareHeader
	midiOutPrepareHeader(fMidiPtrs->midiOut, &header, sizeof(MIDIHDR));

	midiOutLongMsg(fMidiPtrs->midiOut, &header, sizeof(MIDIHDR));

	while (midiOutUnprepareHeader(fMidiPtrs->midiOut, &header, sizeof(MIDIHDR)) == MIDIERR_STILLPLAYING);
}

// # pragma mark - QMidiIn

struct NativeMidiInInstances {
	//! \brief midiIn is a reference to the MIDI input device
	HMIDIIN midiIn;
	//! \brief header is a prepared MIDI header, used for receiving
	//! MIM_LONGDATA (System Exclusive) messages.
	MIDIHDR header;
};

QMap<QString, QString> QMidiIn::devices()
{
	QMap<QString, QString> ret;

	unsigned int numDevs = midiInGetNumDevs();
	if (numDevs == 0)
		return ret;

	for (unsigned int i = 0; i < numDevs; i++) {
		MIDIINCAPSW devCaps;
		midiInGetDevCapsW(i, &devCaps, sizeof(MIDIINCAPSW));
		ret.insert(QString::number(i), QString::fromWCharArray(devCaps.szPname));
	}

	return ret;
}

static void CALLBACK QMidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	QMidiIn* self = reinterpret_cast<QMidiIn*>(dwInstance);
	switch (wMsg)
	{
	case MIM_OPEN:
	case MIM_CLOSE:
		break;
	case MIM_DATA:
		emit(self->midiEvent(static_cast<quint32>(dwParam1), static_cast<quint32>(dwParam2)));
		break;
	case MIM_LONGDATA:
	{
		auto midiHeader = reinterpret_cast<MIDIHDR*>(dwParam1);
		auto rawData = QByteArray(midiHeader->lpData, static_cast<int>(midiHeader->dwBytesRecorded));
		emit(self->midiSysExEvent(rawData));

		// Prepare the midi header to be reused -- what's the worst that could happen?
		midiInUnprepareHeader(hMidiIn, midiHeader, sizeof(MIDIHDR));
		midiInPrepareHeader(hMidiIn, midiHeader, sizeof(MIDIHDR));
		midiInAddBuffer(hMidiIn, midiHeader, sizeof(MIDIHDR));
		break;
	}
	default:
		qWarning("QMidi_Win32: no handler for message %d", wMsg);
	}
}

bool QMidiIn::connect(QString inDeviceId)
{
	if (fConnected)
		disconnect();
	fMidiPtrs = new NativeMidiInInstances;

	fDeviceId = inDeviceId;
	midiInOpen(&fMidiPtrs->midiIn,
		inDeviceId.toInt(),
		reinterpret_cast<DWORD_PTR>(&QMidiInProc),
		reinterpret_cast<DWORD_PTR>(this),
		CALLBACK_FUNCTION | MIDI_IO_STATUS);

	memset(&fMidiPtrs->header, 0, sizeof(MIDIHDR));
	fMidiPtrs->header.lpData = new char[512];  // 512 bytes ought to be enough for everyone
	fMidiPtrs->header.dwBufferLength = 512;
	midiInPrepareHeader(fMidiPtrs->midiIn, &fMidiPtrs->header, sizeof(MIDIHDR));
	midiInAddBuffer(fMidiPtrs->midiIn, &fMidiPtrs->header, sizeof(MIDIHDR));

	fConnected = true;
	return true;
}

void QMidiIn::disconnect()
{
	if (!fConnected)
		return;

	delete fMidiPtrs->header.lpData;
	midiInClose(fMidiPtrs->midiIn);
	fConnected = false;
	delete fMidiPtrs;
	fMidiPtrs = nullptr;
}

void QMidiIn::start()
{
	if (!fConnected)
		return;

	midiInStart(fMidiPtrs->midiIn);
}

void QMidiIn::stop()
{
	if (!fConnected)
		return;

	midiInStop(fMidiPtrs->midiIn);
}

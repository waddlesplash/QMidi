/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"
#include "QMidiIn.h"
#include "OS/QMidi_Haiku.h"

#include <MidiRoster.h>
#include <MidiConsumer.h>
#include <MidiProducer.h>

// # pragma mark - QMidiOut

struct NativeMidiOutInstances {
	BMidiConsumer* midiOutConsumer;
	BMidiLocalProducer* midiOutLocProd;
};

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
	QMap<QString, QString> ret;

	bool OK = true;
	int32 id = 0;
	while (OK) {
		BMidiConsumer* c = BMidiRoster::NextConsumer(&id);
		if (c != NULL) {
			ret.insert(QString::number(id), QString::fromUtf8(c->Name()));
			c->Release();
		} else {
			OK = false;
		}
	}

	return ret;
}

bool QMidiOut::connect(QString outDeviceId)
{
	if (fConnected)
		disconnect();
	fMidiPtrs = new NativeMidiOutInstances;

	fMidiPtrs->midiOutConsumer = BMidiRoster::FindConsumer(outDeviceId.toInt());
	if (fMidiPtrs->midiOutConsumer == NULL) {
		return false;
	}
	fMidiPtrs->midiOutLocProd = new BMidiLocalProducer("QMidi");
	if (!fMidiPtrs->midiOutLocProd->IsValid()) {
		fMidiPtrs->midiOutLocProd->Release();
		return false;
	}
	fMidiPtrs->midiOutLocProd->Register();
	if (fMidiPtrs->midiOutLocProd->Connect(fMidiPtrs->midiOutConsumer) != B_OK) {
		return false;
	}

	fDeviceId = outDeviceId;
	fConnected = true;
	return true;
}

void QMidiOut::disconnect()
{
	if (!fConnected)
		return;

	fMidiPtrs->midiOutLocProd->Disconnect(fMidiPtrs->midiOutConsumer);
	fMidiPtrs->midiOutConsumer->Release();
	fMidiPtrs->midiOutLocProd->Unregister();
	fMidiPtrs->midiOutLocProd->Release();
	fConnected = false;

	delete fMidiPtrs;
	fMidiPtrs = NULL;
}

void QMidiOut::sendMsg(qint32 msg)
{
	if (!fConnected)
		return;

	uchar command = msg & 0xF0;
	uchar channel = msg & 0x0F;
	uchar lsb = (msg >> 8) & 0xFF;
	uchar msb = (msg >> 16) & 0xFF;

	switch (command)
	{
	case 0x80:
		fMidiPtrs->midiOutLocProd->SprayNoteOff(channel, lsb, msb);
		break;
	case 0x90:
		fMidiPtrs->midiOutLocProd->SprayNoteOn(channel, lsb, msb);
		break;
	case 0xA0:
		fMidiPtrs->midiOutLocProd->SprayKeyPressure(channel, lsb, msb);
		break;
	case 0xB0:
		fMidiPtrs->midiOutLocProd->SprayControlChange(channel, lsb, msb);
		break;
	case 0xC0:
		fMidiPtrs->midiOutLocProd->SprayProgramChange(channel, lsb);
		break;
	case 0xD0:
		fMidiPtrs->midiOutLocProd->SprayChannelPressure(channel, lsb);
		break;
	case 0xE0:
		fMidiPtrs->midiOutLocProd->SprayPitchBend(channel, lsb, msb);
		break;
	default:
		qWarning("QMidiOut::sendMsg: unknown command %02x", command);
	}
}

void QMidiOut::sendSysEx(const QByteArray &data)
{
	if (!fConnected)
		return;

	// SpraySystemExclusive expects the payload without the 0xF0 and 0xF7 markers only
	if (!(data.front() == '\xF0' && data.back() == '\xF7')) {
		qWarning("QMidiOut::sendSysEx: invalid SysEx data passed");
		return;
	}
	char* payload = const_cast<char*>(data.constData()) + 1;
	size_t payloadLength = data.length() - 2;

	fMidiPtrs->midiOutLocProd->SpraySystemExclusive(payload, payloadLength);
}

// # pragma mark - QMidiIn

struct NativeMidiInInstances {
	BMidiProducer* midiInProducer;
	QMidiInternal::MidiInConsumer* midiInConsumer;
};

QMap<QString, QString> QMidiIn::devices()
{
	QMap<QString, QString> ret;

	bool OK = true;
	int32 id = 0;
	while (OK) {
		BMidiProducer* c = BMidiRoster::NextProducer(&id);
		if (c != NULL) {
			ret.insert(QString::number(id), QString::fromUtf8(c->Name()));
			c->Release();
		} else {
			OK = false;
		}
	}

	return ret;
}

bool QMidiIn::connect(QString inDeviceId)
{
	if (fConnected)
		disconnect();

	fMidiPtrs = new NativeMidiInInstances;

	fMidiPtrs->midiInProducer = BMidiRoster::FindProducer(inDeviceId.toInt());
	if (fMidiPtrs->midiInProducer == NULL) {
		return false;
	}
	fMidiPtrs->midiInConsumer = new QMidiInternal::MidiInConsumer(this, "QMidi");
	if (!fMidiPtrs->midiInConsumer->IsValid()) {
		fMidiPtrs->midiInConsumer->Release();
		return false;
	}
	fMidiPtrs->midiInConsumer->Register();

	fDeviceId = inDeviceId;
	fConnected = true;

	return true;
}

void QMidiIn::disconnect()
{
	if (!fConnected)
		return;

	stop();

	fMidiPtrs->midiInConsumer->Release();
	fMidiPtrs->midiInConsumer->Unregister();
	fMidiPtrs->midiInProducer->Release();

	fConnected = false;
	delete fMidiPtrs;
	fMidiPtrs = NULL;
}

void QMidiIn::start()
{
	if (!fConnected)
		return;

	if (fMidiPtrs->midiInProducer->Connect(fMidiPtrs->midiInConsumer) != B_OK) {
		qWarning("QMidiIn::start: could not connect producer with our consumer");
		return;
	}
}

void QMidiIn::stop()
{
	if (!fConnected)
		return;

	fMidiPtrs->midiInProducer->Disconnect(fMidiPtrs->midiInConsumer);
}

QMidiInternal::MidiInConsumer::MidiInConsumer(QMidiIn* midiIn, const char* name)
	: BMidiLocalConsumer(name), fMidiIn(midiIn)
{
}

void QMidiInternal::MidiInConsumer::ChannelPressure(uchar channel, uchar pressure, bigtime_t time)
{
	int data = 0xD0
			| (channel & 0x0F)
			| (pressure << 8);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::ControlChange(uchar channel, uchar controlNumber, uchar controlValue, bigtime_t time)
{
	int data = 0xB0
			| (channel & 0x0F)
			| (controlNumber << 8)
			| (controlValue << 16);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::KeyPressure(uchar channel, uchar note, uchar pressure, bigtime_t time)
{
	int data = 0xA0
			| (channel & 0x0F)
			| (note << 8)
			| (pressure << 16);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::NoteOff(uchar channel, uchar note, uchar velocity, bigtime_t time)
{
	int data = 0x80
			| (channel & 0x0F)
			| (note << 8)
			| (velocity << 16);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time)
{
	int data = 0x90
			| (channel & 0x0F)
			| (note << 8)
			| (velocity << 16);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::PitchBend(uchar channel, uchar lsb, uchar msb, bigtime_t time)
{
	int data = 0xE0
			| (channel & 0x0F)
			| (lsb << 8)
			| (msb << 16);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::ProgramChange(uchar channel, uchar programNumber, bigtime_t time)
{
	int data = 0xC0
			| (channel & 0x0F)
			| (programNumber << 8);
	emit(fMidiIn->midiEvent(static_cast<quint32>(data), time));
}

void QMidiInternal::MidiInConsumer::SystemExclusive(void* data, size_t length, bigtime_t time)
{
	QByteArray ba = QByteArray(reinterpret_cast<const char*>(data), length);
	ba.prepend('\xF0');
	ba.append('\xF7');
	emit(fMidiIn->midiSysExEvent(ba));
}

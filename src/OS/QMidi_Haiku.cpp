/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include <MidiRoster.h>
#include <MidiConsumer.h>
#include <MidiProducer.h>

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

	char buf[3];
	buf[0] = msg & 0xFF;
	buf[1] = (msg >> 8) & 0xFF;
	buf[2] = (msg >> 16) & 0xFF;

	fMidiPtrs->midiOutLocProd->SprayData((void*)&buf, 3, true);
}

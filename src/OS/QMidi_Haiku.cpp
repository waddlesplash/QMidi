/*
 * Copyright 2012-2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include <MidiRoster.h>
#include <MidiConsumer.h>
#include <MidiProducer.h>
struct MidiPtrObjs {
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
	if (myConnected)
		disconnect();
	myMidiPtrs = new MidiPtrObjs;

	myMidiPtrs->midiOutConsumer = BMidiRoster::FindConsumer(outDeviceId.toInt());
	if (myMidiPtrs->midiOutConsumer == NULL) {
		return false;
	}
	myMidiPtrs->midiOutLocProd = new BMidiLocalProducer("QMidi");
	if (!myMidiPtrs->midiOutLocProd->IsValid()) {
		myMidiPtrs->midiOutLocProd->Release();
		return false;
	}
	myMidiPtrs->midiOutLocProd->Register();
	if (myMidiPtrs->midiOutLocProd->Connect(myMidiPtrs->midiOutConsumer) != B_OK) {
		return false;
	}

	myDeviceId = outDeviceId;
	myConnected = true;
	return true;
}

void QMidiOut::disconnect()
{
	if (!myConnected) {
		return;
	}

	myMidiPtrs->midiOutLocProd->Disconnect(myMidiPtrs->midiOutConsumer);
	myMidiPtrs->midiOutConsumer->Release();
	myMidiPtrs->midiOutLocProd->Unregister();
	myMidiPtrs->midiOutLocProd->Release();

	delete myMidiPtrs;
	myMidiPtrs = NULL;
	myConnected = false;
}

void QMidiOut::sendMsg(qint32 msg)
{
	if (!myConnected) {
		return;
	}

	char buf[3];
	buf[0] = msg & 0xFF;
	buf[1] = (msg >> 8) & 0xFF;
	buf[2] = (msg >> 16) & 0xFF;

	myMidiPtrs->midiOutLocProd->SprayData((void*)&buf, 3, true);
}

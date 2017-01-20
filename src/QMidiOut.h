/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <QMap>
#include <QString>

class QMidiEvent;
struct NativeMidiOutInstances;

class QMidiOut
{
public:
	static QMap<QString /* key */, QString /* name */> devices();

	QMidiOut();
	~QMidiOut();
	bool connect(QString outDeviceId);
	void disconnect();
	void sendMsg(qint32 msg);

	void sendEvent(const QMidiEvent& e);
	void setInstrument(int voice, int instr);
	void noteOn(int note, int voice, int velocity = 64);
	void noteOff(int note, int voice);
	void pitchWheel(int voice, int value);
	void channelAftertouch(int voice, int value);
	void polyphonicAftertouch(int note, int voice, int value);
	void controlChange(int voice, int number, int value);
	void stopAll();
	void stopAll(int voice);

private:
	QString fDeviceId;
	NativeMidiOutInstances* fMidiPtrs;
	bool fConnected;
};

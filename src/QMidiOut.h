/*
 * Copyright 2012-2015 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#pragma once

#include <QObject>
#include <QMap>
#include <QString>

class QMidiEvent;
struct NativeMidiInstances;

class QMidiOut : public QObject
{
	Q_OBJECT
public:
	static QMap<QString, QString> devices();

	QMidiOut(QObject* parent = 0);
	bool connect(QString outDeviceId);
	void disconnect();
	void sendMsg(qint32 msg);

	void sendEvent(QMidiEvent& e);
	void setInstrument(int voice, int instr);
	void noteOn(int note, int voice, int velocity = 64);
	void noteOff(int note, int voice);
	void pitchWheel(int voice, int value);
	void controlChange(int voice, int number, int value);
	void stopAll();
	void stopAll(int voice);

signals:
	void allNotesStopped(int voice);

private:
	QString fDeviceId;
	NativeMidiInstances* fMidiPtrs;
	bool fConnected;
};

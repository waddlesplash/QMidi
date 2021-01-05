/*
 * Copyright 2020 Jacob Secunda <secundaja@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"
#include "QMidiIn.h"

#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>

// # pragma mark - QMidiOut

struct NativeMidiOutInstances {
	MIDIClientRef client;
	MIDIPortRef outputPort;
	MIDIEndpointRef destinationId;
};

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
	QMap<QString, QString> ret;

	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
	int destinations = MIDIGetNumberOfDestinations();
	for (int destIndex = 0; destIndex <= destinations; destIndex++) {
		MIDIEndpointRef destRef = MIDIGetDestination(destIndex);
		if (destRef != 0) {
			CFStringRef stringRef = nullptr;
  			char name[256];

  			MIDIObjectGetStringProperty(destRef,
				kMIDIPropertyDisplayName, &stringRef);
			CFStringGetCString(stringRef, name, sizeof(name),
				kCFStringEncodingUTF8);
			CFRelease(stringRef);

			ret.insert(QString::number(destIndex),
				QString::fromUtf8(name));
		}
	}

	return ret;
}

bool QMidiOut::connect(QString outDeviceId)
{
	if (fConnected)
		disconnect();

	OSStatus result;
	fMidiPtrs = new NativeMidiOutInstances;

	QString name = "QMidi Output Client";
	result = MIDIClientCreate(name.toCFString(), nullptr, nullptr,
		&fMidiPtrs->client);
	if (result != noErr)
		return false;

	QString portName = "QMidi Output Port " + outDeviceId;
	result = MIDIOutputPortCreate(fMidiPtrs->client, portName.toCFString(),
		&fMidiPtrs->outputPort);
	if (result != noErr) {
		MIDIClientDispose(fMidiPtrs->client);
		return false;
	}

	fMidiPtrs->destinationId = MIDIGetDestination(outDeviceId.toInt());
	if (fMidiPtrs->destinationId == 0) {
		MIDIPortDispose(fMidiPtrs->outputPort);
		MIDIClientDispose(fMidiPtrs->client);
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

	if (fMidiPtrs->destinationId != 0) {
		MIDIEndpointDispose(fMidiPtrs->destinationId);
		fMidiPtrs->destinationId = 0;
	}

	if (fMidiPtrs->outputPort != 0) {
		MIDIPortDispose(fMidiPtrs->outputPort);
		fMidiPtrs->outputPort = 0;
	}

	if (fMidiPtrs->client != 0) {
		MIDIClientDispose(fMidiPtrs->client);
		fMidiPtrs->client = 0;
	}

	fConnected = false;

	delete fMidiPtrs;
	fMidiPtrs = 0;
}

void QMidiOut::sendMsg(qint32 msg)
{
	if (!fConnected)
		return;

	MIDIPacketList packetList;
	MIDIPacket *packet = MIDIPacketListInit(&packetList);

	MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
	packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet,
		timeStamp, sizeof(msg), (Byte*)&msg);

	MIDISend(fMidiPtrs->outputPort, fMidiPtrs->destinationId, &packetList);
}

void QMidiOut::sendSysEx(const QByteArray &data)
{
	if (!fConnected)
		return;

	MIDISysexSendRequest request;
	request.bytesToSend = data.length();
	request.complete = false;
	request.completionProc = nullptr;
	request.completionRefCon = nullptr;
	request.data = (Byte *)data.constData();
	request.destination = fMidiPtrs->destinationId;

	MIDISendSysex(&request);
}

// # pragma mark - QMidiIn

struct NativeMidiInInstances {
	MIDIClientRef client;
	MIDIPortRef inputPort;
	MIDIEndpointRef sourceId;
};

static void QMidiInReadProc(const MIDIPacketList *list, void *readProc,
	void *srcConn)
{
	QMidiIn *midiIn = static_cast<QMidiIn *>(readProc);
	MIDIPacket *packet = const_cast<MIDIPacket *>(list->packet);

	for (UInt32 index = 0; index < list->numPackets; index++) {
		UInt16 byteCount = packet->length;

		// Check that the MIDIPacket has data, and is a normal midi
		// message. (We don't support Sysex, status, etc for CoreMIDI at
		// the moment.)
		if (byteCount != 0
			&& packet->data[0] < 0xF0
			&& (packet->data[0] & 0x80) != 0x00) {
			emit(midiIn->midiEvent(*packet->data,
				packet->timeStamp));
	    	}

		packet = MIDIPacketNext(packet);
	}
}

QMap<QString, QString> QMidiIn::devices()
{
	QMap<QString, QString> ret;

	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
	int sources = MIDIGetNumberOfSources();
	for (int sourceIndex = 0; sourceIndex <= sources; sourceIndex++) {
		MIDIEndpointRef sourceRef = MIDIGetSource(sourceIndex);
		if (sourceRef != 0) {
			CFStringRef stringRef = 0;
			char name[256];

			MIDIObjectGetStringProperty(sourceRef,
				kMIDIPropertyDisplayName, &stringRef);
			CFStringGetCString(stringRef, name, sizeof(name),
				kCFStringEncodingUTF8);
			CFRelease(stringRef);

			ret.insert(QString::number(sourceIndex),
				QString::fromUtf8(name));
		}
	}

	return ret;
}

bool QMidiIn::connect(QString inDeviceId)
{
	if (fConnected)
		disconnect();

	OSStatus result;
	fMidiPtrs = new NativeMidiInInstances;

	QString name = "QMidi Input Client";
	result = MIDIClientCreate(name.toCFString(), nullptr, nullptr,
		&fMidiPtrs->client);
	if (result != noErr)
		return false;

	QString portName = "QMidi Input Port " + inDeviceId;
	result = MIDIInputPortCreate(fMidiPtrs->client, portName.toCFString(),
		QMidiInReadProc, this, &fMidiPtrs->inputPort);
	if (result != noErr) {
		MIDIClientDispose(fMidiPtrs->client);
		return false;
	}

	fMidiPtrs->sourceId = MIDIGetSource(inDeviceId.toInt());
	if (fMidiPtrs->sourceId == 0) {
		MIDIPortDispose(fMidiPtrs->inputPort);
		MIDIClientDispose(fMidiPtrs->client);
		return false;
	}

	fDeviceId = inDeviceId;
	fConnected = true;
	return true;
}

void QMidiIn::disconnect()
{
	if (!fConnected)
		return;

	MIDIPortDisconnectSource(fMidiPtrs->inputPort, fMidiPtrs->sourceId);

	if (fMidiPtrs->inputPort != 0) {
		MIDIPortDispose(fMidiPtrs->inputPort);
		fMidiPtrs->inputPort = 0;
	}

	if (fMidiPtrs->client != 0) {
		MIDIClientDispose(fMidiPtrs->client);
		fMidiPtrs->client = 0;
	}

	fConnected = false;

	delete fMidiPtrs;
	fMidiPtrs = nullptr;
}

void QMidiIn::start()
{
	if (!fConnected)
		return;

	MIDIPortConnectSource(fMidiPtrs->inputPort, fMidiPtrs->sourceId,
		nullptr);
}

void QMidiIn::stop()
{
	if (!fConnected)
		return;

	MIDIPortDisconnectSource(fMidiPtrs->inputPort, fMidiPtrs->sourceId);
}

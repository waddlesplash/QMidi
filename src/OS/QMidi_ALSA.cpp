/*
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "QMidiOut.h"

#include <QStringList>
#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_midi_event.h>

struct NativeMidiOutInstances {
	snd_seq_t* midiOutPtr;
};

// TODO: error reporting

QMap<QString, QString> QMidiOut::devices()
{
	QMap<QString, QString> ret;

	snd_seq_client_info_t* cinfo;
	snd_seq_port_info_t* pinfo;
	int client;
	int err;
	snd_seq_t* handle;

	err = snd_seq_open(&handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0) {
		/* Use snd_strerror(errno) to get the error here. */
		return ret;
	}

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_client_info_set_client(cinfo, -1);

	while (snd_seq_query_next_client(handle, cinfo) >= 0) {
		client = snd_seq_client_info_get_client(cinfo);
		snd_seq_port_info_alloca(&pinfo);
		snd_seq_port_info_set_client(pinfo, client);

		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(handle, pinfo) >= 0) {
			int cap = (SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE);
			if ((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
				QString port = QString::number(snd_seq_port_info_get_client(pinfo));
				port += ":" + QString::number(snd_seq_port_info_get_port(pinfo));
				QString name = snd_seq_client_info_get_name(cinfo);
				ret.insert(port, name);
			}
		}
	}

	return ret;
}

bool QMidiOut::connect(QString outDeviceId)
{
	if (fConnected)
		disconnect();
	fMidiPtrs = new NativeMidiOutInstances;

	int err = snd_seq_open(&fMidiPtrs->midiOutPtr, "default", SND_SEQ_OPEN_OUTPUT, 0);
	if (err < 0)
		return false;
	snd_seq_set_client_name(fMidiPtrs->midiOutPtr, "QMidi");

	snd_seq_create_simple_port(fMidiPtrs->midiOutPtr, "Output Port", SND_SEQ_PORT_CAP_READ,
		SND_SEQ_PORT_TYPE_MIDI_GENERIC);

	QStringList l = outDeviceId.split(":");
	int client = l.at(0).toInt();
	int port = l.at(1).toInt();
	snd_seq_connect_to(fMidiPtrs->midiOutPtr, 0, client, port);

	fDeviceId = outDeviceId;
	fConnected = true;
	return true;
}

void QMidiOut::disconnect()
{
	if (!fConnected)
		return;

	QStringList l = fDeviceId.split(":");
	int client = l.at(0).toInt();
	int port = l.at(1).toInt();

	snd_seq_disconnect_from(fMidiPtrs->midiOutPtr, 0, client, port);
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

	snd_seq_event_t ev;
	snd_midi_event_t* mev;

	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, 0);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	snd_midi_event_new(3, &mev);
	snd_midi_event_resize_buffer(mev, 3);
	snd_midi_event_encode(mev, (unsigned char*)&buf, 3, &ev);

	snd_seq_event_output(fMidiPtrs->midiOutPtr, &ev);
	snd_seq_drain_output(fMidiPtrs->midiOutPtr);
}

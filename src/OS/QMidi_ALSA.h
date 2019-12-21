/*
 * Copyright 2019 Georg Gadinger <nilsding@nilsding.org>
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <QThread>

class QMidiIn;
struct NativeMidiInInstances;

namespace QMidiInternal
{
//! \brief The MidiInReceiveThread class runs the ALSA MIDI input thread.  This
//! is required since the function that reads input from a \c snd_seq_t handle
//! blocks.
class MidiInReceiveThread : public QThread
{
	Q_OBJECT

public:
	MidiInReceiveThread(QMidiIn* qMidiIn, NativeMidiInInstances* fMidiPtrs, QObject* parent = nullptr);

private:
	void run() override;

private:
	QMidiIn* fMidiIn;
	NativeMidiInInstances* fMidiPtrs;
};
};

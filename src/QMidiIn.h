/*
 * Copyright 2019 Georg Gadinger <nilsding@nilsding.org>
 * Distributed under the terms of the MIT license.
 */
#pragma once

#include <QMap>
#include <QString>
#include <QObject>

struct NativeMidiInInstances;

class QMidiIn : public QObject
{
	Q_OBJECT
public:
	//! \brief Devices returns a list of MIDI input devices.
	//!
	//! The items in the returned QMap specify the internal device ID as the
	//! \c key which needs to be used with QMidiIn::connect, whereas the
	//! \c value contains the device name that can be used e.g. in a QComboBox.
	//! \return QMap<device ID, human-readable device name>
	static QMap<QString /* key */, QString /* name */> devices();

	explicit QMidiIn(QObject *parent = nullptr);
	~QMidiIn();

	//! \brief connect Connect to the MIDI input device specified by
	//! <tt>inDeviceId</tt>.
	//! \param inDeviceId The device ID, as returned by QMidiIn::devices.
	//! \return \c true if the connection was successful, \c false otherwise.
	bool connect(QString inDeviceId);
	//! \brief disconnect Disconnect the previously connected MIDI input device.
	void disconnect();

	//! \brief start Starts listening for input from the device.
	void start();
	//! \brief stop Stops listening for input from the device.
	void stop();

	//! \brief isConnected Returns whether the device is connected or not.
	//! \return \c true if the input device is connected, \c false otherwise.
	bool isConnected() const { return fConnected; }
	//! \brief deviceId Returns the device ID used with QMidiIn::connect.
	//! \return The device ID used to connect with.
	QString deviceId() const { return fDeviceId; }

signals:
	//! \brief midiEvent This signal is emitted when a basic MIDI event is
	//! received.
	//!
	//! You can use QMidiEvent::setMessage for easier handling of the event.
	//! For example:
	//! \code{.cpp}
	//! void MainWindow::onMidiEvent(quint32 message, quint32 timing)
	//! {
	//!     QMidiEvent event;
	//!     event.setMessage(message);
	//!
	//!     qDebug() << "received event" << event.type()
	//!              << "note:" << event.note()
	//!              << "velocity:" << event.velocity();
	//! }
	//! \endcode
	//! \param message The received MIDI message.
	//! \param timing Timing information provided by operating system.
	void midiEvent(quint32 message, quint32 timing);
	//! \brief midiSysExEvent This signal is emitted when a MIDI System
	//! Exclusive (SysEx) event is received.
	//! \param data The received SysEx data, including the SysEx start (0xF0)
	//! and end bytes (0xF7).
	void midiSysExEvent(QByteArray data);

private:
	QString fDeviceId;
	NativeMidiInInstances* fMidiPtrs;
	bool fConnected;
};

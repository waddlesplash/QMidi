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
    ///
    /// \brief devices returns a list of MIDI input devices
    /// \return QMap<device ID, human-readable device name>
    ///
    static QMap<QString /* key */, QString /* name */> devices();

    explicit QMidiIn(QObject *parent = nullptr);
    ~QMidiIn();

    bool connect(QString inDeviceId);
    void disconnect();

    ///
    /// \brief start starts listening for input from the device.
    ///
    void start();
    ///
    /// \brief stop stops listening for input from the device.
    ///
    void stop();

    bool isConnected() const { return fConnected; }
    QString deviceId() const { return fDeviceId; }

signals:
    void midiEvent(quint32 message, quint32 timing);
    void midiSysExEvent(QByteArray data);

private:
    QString fDeviceId;
    NativeMidiInInstances* fMidiPtrs;
    bool fConnected;
};

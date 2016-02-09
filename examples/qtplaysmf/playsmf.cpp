/*
 * Copyright 2003-2012 by David G. Slomin
 * Copyright 2012-2016 Augustin Cavalier <waddlesplash>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <stdio.h>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

#include <QMidiOut.h>
#include <QMidiFile.h>

class MidiPlayer : public QThread
{
	Q_OBJECT
public:
	MidiPlayer(QMidiFile* file, QMidiOut* out)
	{
		midi_file = file;
		midi_out = out;
	}

private:
	QMidiFile* midi_file;
	QMidiOut* midi_out;

protected:
	void run()
	{
		QElapsedTimer t;
		t.start();
		QList<QMidiEvent*> events = midi_file->events();
		for (QMidiEvent* e : events) {
			if (e->type() != QMidiEvent::Meta) {
				qint64 event_time = midi_file->timeFromTick(e->tick()) * 1000;

				qint32 waitTime = event_time - t.elapsed();
				if (waitTime > 0) {
					msleep(waitTime);
				}
				if (e->type() == QMidiEvent::SysEx) {
					// TODO: sysex
				} else {
					qint32 message = e->message();
					midi_out->sendMsg(message);
				}
			}
		}

		midi_out->disconnect();
	}
};

static void usage(char* program_name)
{
	fprintf(stderr, "Usage: %s -p<port> <MidiFile>\n\n", program_name);
	fputs("Ports:\nID\tName\n----------------\n", stderr);
	QMap<QString, QString> vals = QMidiOut::devices();
	for (QString key : vals.keys()) {
		QString value = vals.value(key);
		fputs(key.toUtf8().constData(), stderr);
		fputs("\t", stderr);
		fputs(value.toUtf8().constData(), stderr);
		fputs("\n", stderr);
	}
	exit(1);
}

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QString filename = "";
	QString midiOutName = "";
	QMidiFile* midi_file = new QMidiFile();

	for (int i = 1; i < argc; i++) {
		QString curArg(argv[i]);
		if ((curArg == "--help") || (curArg == "-h") || (curArg == "/?") || (curArg == "/help")) {
			usage(argv[0]);
		} else if (curArg.startsWith("-p")) {
			midiOutName = curArg.mid(2);
		} else if (filename == "") {
			filename = argv[i];
		} else {
			usage(argv[0]);
		}
	}

	if ((filename == "") || (midiOutName == "")) {
		usage(argv[0]);
	}
	midi_file->load(filename);

	QMidiOut* midi_out = new QMidiOut();
	midi_out->connect(midiOutName);

	MidiPlayer* p = new MidiPlayer(midi_file, midi_out);
	QObject::connect(p, SIGNAL(finished()), &a, SLOT(quit()));
	p->start();

	return a.exec();
}

#include "playsmf.moc"

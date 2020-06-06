# QMidi [![Build Status](https://travis-ci.org/waddlesplash/QMidi.png)](https://travis-ci.org/waddlesplash/QMidi)
A simple, cross-platform way to support MIDI in Qt. MIDI Output and MIDI File I/O is supported.
MIDI output is supported on Windows (Windows Multimedia), Linux (ALSA), Haiku
(Midi Kit 2), and macOS (CoreMIDI).

## MIDI Output
The `QMidiOut` class provides an interface to the system's MIDI Out.
```cpp
/* devices() returns a QMap where the key is the ID and
 * the value is the user-friendly name. */
QMap<QString, QString> vals = QMidiOut::devices();
QMidiOut midi;
midi.connect(/* one of the keys (IDs) from `devices()` */);
```
There's an easy API for sending messages:
```cpp
midi.setInstrument(/* voice */ 0, /* instrument */ 0);
midi.noteOn(/* note */ 60, /* voice */ 0 /* , velocity */);
midi.noteOff(/* note */ 60, /* voice */ 0);
```
Alternatively, you could just send raw MIDI messages:
```cpp
midi.sendMsg(0x90 + 0 | 60 << 8 | 64 << 16);
/* note on, voice 0; middle C (60); velocity 64 */
```
Or you could construct a QMidiEvent (from `QMidiFile.h`) and send it:
```cpp
QMidiEvent e;
e.setType(QMidiEvent::NoteOn);
e.setVoice(0);
e.setNote(60);
e.setVelocity(64);
midi.sendEvent(e);
```
Once you're done:
```cpp
midi.disconnect();
```

## MIDI File I/O
Classes for MIDI file I/O were rewritten from Div's Midi Utilities
([homepage](http://www.sreal.com/~div/midi-utilities/) | [Google Code](http://code.google.com/p/divs-midi-utilities/))
from C to Qt/C++.

Quick example:
```cpp
QMidiFile f;
f.load(" .. some filename .. ");
f.save(" .. some filename .. ");
```
You can get the events using `f.events()` which returns a `QList<QMidiEvent*>*`.
For information on using these classes in conjuction with the MIDI output class to play files
see the `qtplaysmf` example in the `examples` folder.

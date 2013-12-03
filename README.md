# QMidi [![Build Status](https://travis-ci.org/waddlesplash/qtmidi.png)](https://travis-ci.org/waddlesplash/qtmidi)
A simple, cross-platform way to support MIDI. Classes for MIDI Output and MIDI File I/O are supported.
MIDI output is supported on Windows (via `mmsystem`), Linux (via `ALSA`), and Haiku (via `libmidi2` - EXPERIMENTAL).

## MIDI Output
To output MIDI, simply initialize the `QMidiOut` class...
```cpp
QMap<QString,QString> vals = QMidiOut::devices();
/* outDeviceNames() returns a QMap where the key is the ID and
 * the value is the user-friendly name. */
QMidiOut midi;
midi.connect("key goes here");
```
...and then send messages:
```cpp
midi.setInstr(0, 0);
/* Voice (0-15), Instrument (0-127) */
midi.noteOn(60,0);
/* Note (0-127), Voice (0-15) [, Velocity (0-127)] */
midi.noteOff(60,0);
/* Note (0-127), Voice (0-15) */
```
Alternatively, you could just send MIDI note data:
```cpp
midi.sendMsg(0x90+0 | 60<<8 | 64<<16);
/* note on, voice 0; middle C (60); velocity 64 */
```
Or construct a QMidiEvent (from `QMidiFile.h`) and send it:
```cpp
QMidiEvent* e = new QMidiEvent();
e->setType(QMidiEvent::NoteOn);
e->setVoice(0);
e->setNote(60);
e->setVelocity(64);
midi.sendEvent(e);
```
Before your application closes or if you want to open output on a different port...
```cpp
midi.disconnect();
```

## MIDI File I/O
Classes for MIDI file I/O were rewritten from Div's Midi Utilities ([homepage](http://www.sreal.com/~div/midi-utilities/) | [Google Code](http://code.google.com/p/divs-midi-utilities/)) into Qt/C++.

Use the classes like so:
```cpp
QMidiFile f;
f.load(" .. some filename .. ");
f.save(" .. some filename .. ");
```
You can get the events using `f.events()` which returns a `QList<QMidiEvent*>*`. For information on using these classes in conjuction with the MIDI output class to play files, see the `qtplaysmf` example in the `examples` folder.

# QMidi [![Build Status](https://travis-ci.org/waddlesplash/qtmidi.png)](https://travis-ci.org/waddlesplash/qtmidi)
A simple, cross-platform way to support MIDI. Classes for MIDI Output and MIDI File I/O are supported.
MIDI output is supported on Windows (via `mmsystem`), Linux (via `ALSA`), and Haiku (via `libmidi2` - EXPERIMENTAL).

## MIDI Output
To output MIDI, simply initialize the `QMidi` class...
```cpp
QMap<QString,QString> vals = QMidi::outDeviceNames();
// outDeviceNames() returns a QMap where the key is the ID and
// the value is the user-friendly name.
QMidi::initMidiOut(".. some key from the vals above ..");
```
...and then send messages:
```cpp
QMidi::outSetInstr(0, 0);
/* Voice (0-15), Instrument (0-127) */
QMidi::outNoteOn(60,0);
/* Note (0-127), Voice (0-15) [, Velocity (0-127)] */
QMidi::outNoteOff(60,0);
/* Note (0-127), Voice (0-15) */
```
Alternatively, you could just send MIDI note data:
```cpp
QMidi::outSendMsg(0x90+0 | 60<<8 | 64<<16);
/* note on, voice 0; middle C (60); velocity 64 */
```
Before your application closes or if you want to open output on a different port...
```cpp
QMidi::closeMidiOut();
```

As you can see from the examples, all functions in the `QMidi` class are static and you do not need to create an object.

## MIDI File I/O
Classes for MIDI file I/O were rewritten from Div's Midi Utilities ([homepage](http://www.sreal.com/~div/midi-utilities/) | [Google Code](http://code.google.com/p/divs-midi-utilities/)) into Qt/C++.

Use the classes like so:
```cpp
QMidiFile f;
f.load(" .. some filename .. ");
f.save(" .. some filename .. ");
```
You can get the events using `f.events()` which returns a `QList<QMidiEvent*>`. For information on using these classes in conjuction with the MIDI output class to play files, see the `qtplaysmf` example in the `examples` folder.

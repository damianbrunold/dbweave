/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Thin wrapper around QSerialPort matching the legacy
    SerialPort helper from comutil.h. Preserves the GetChar /
    PurgeInput / Send / Receive API so the ported controllers read
    and write to the port using the same idioms as the legacy
    code.                                                         */

#ifndef DBWEAVE_LOOM_SERIAL_PORT_H
#define DBWEAVE_LOOM_SERIAL_PORT_H

#include <QByteArray>
#include <QString>

#include "loomsettings.h"   /* PORT / PORTINIT enums */

class QSerialPort;

class SerialPort
{
public:
	SerialPort ();
	~SerialPort ();

	/*  Opens the given COM port with the supplied init block. The
	    PORT enum maps 1..8 → COM1..COM8 on Windows and /dev/ttyS0..
	    /dev/ttyS7 on Linux; the wrapper picks the platform-native
	    name. */
	bool Open (PORT _port, const PORTINIT& _init);
	bool IsOpen () const;
	void Close ();

	/*  NUL-terminated send. Blocks until all bytes are flushed or
	    a write error occurs. */
	bool Send (const char* _buffer);
	bool Send (const char* _buffer, int _length);

	/*  Reads up to _length bytes into _buffer. On timeout, returns
	    whatever came in so far; legacy callers only pop one char
	    at a time via GetChar anyway. */
	bool Receive (char* _buffer, int _length);

	/*  Returns the next buffered byte, or '\0' if nothing is
	    available. Maintains an internal QByteArray so the caller
	    can poll a byte-stream byte-by-byte (matching legacy
	    TComPort::InputCount/ReadChar). */
	char GetChar ();

	void PurgeInput ();

private:
	QSerialPort* port = nullptr;
	QByteArray   rxBuf;

	/*  Pulls any available bytes from the OS into rxBuf, waiting
	    up to _waitMs for the first byte. */
	void drainInto (int _waitMs);

	/*  Resolve a PORT enum to a platform-native device name
	    (COM* on Windows, /dev/ttyS* or /dev/ttyUSB* on Linux). */
	static QString portName (PORT _p);
};

#endif

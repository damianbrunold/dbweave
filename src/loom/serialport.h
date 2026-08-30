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
    code.

    Two legacy transport behaviours that the first Qt port dropped
    are reinstated here because the loom controllers depend on
    them:

      - Send() paces the line one character at a time with a 10 ms
        gap, exactly as legacy comutil.cpp did via TransmitChar +
        Sleep(10).
      - GetChar() polls rather than blocking; legacy read straight
        out of TComPort's input queue and returned '\0' when it was
        empty.

    Open() additionally re-asserts DTR/RTS. Qt's Windows backend
    forces RTS_CONTROL_DISABLE whenever flow control is not
    hardware handshaking, which the Winsoft TComPort component
    never did -- interfaces that take their enable level off RTS
    see nothing without this.                                     */

#ifndef DBWEAVE_LOOM_SERIAL_PORT_H
#define DBWEAVE_LOOM_SERIAL_PORT_H

#include <QByteArray>
#include <QString>

#include "loomsettings.h" /* PORTINIT */

class QSerialPort;

class SerialPort
{
public:
    SerialPort();
    ~SerialPort();

    /*  Opens the named device with the supplied init block.
        _portName is the platform-native name as chosen in the loom
        options dialog ("COM3", "/dev/ttyUSB0", "/dev/cu.usbserial-A1"
        ...); QSerialPort takes care of the \\.\ prefixing that
        Windows needs above COM9. */
    bool Open(const QString& _portName, const PORTINIT& _init);
    bool IsOpen() const;
    void Close();

    /*  Whether Open() should raise DTR and RTS once the device is
        open. Defaults to true (the legacy line state). Must be set
        before Open(). */
    void SetAssertLines(bool _on)
    {
        assertLines = _on;
    }

    /*  NUL-terminated send. Paced at one character per
        SEND_CHAR_DELAY_MS, like legacy. */
    bool Send(const char* _buffer);
    bool Send(const char* _buffer, int _length);

    /*  Reads up to _length bytes into _buffer. On timeout, returns
        whatever came in so far; legacy callers only pop one char
        at a time via GetChar anyway. */
    bool Receive(char* _buffer, int _length);

    /*  Returns the next buffered byte, or '\0' if nothing is
        available. Maintains an internal QByteArray so the caller
        can poll a byte-stream byte-by-byte (matching legacy
        TComPort::InputCount/ReadChar). */
    char GetChar();

    void PurgeInput();

private:
    QSerialPort* port = nullptr;
    QByteArray rxBuf;
    bool assertLines = true;

    /*  Pulls any available bytes from the OS into rxBuf, waiting
        up to _waitMs for the first byte. */
    void drainInto(int _waitMs);
};

#endif

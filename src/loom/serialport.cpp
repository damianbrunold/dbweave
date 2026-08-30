/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "serialport.h"
#include "loomlog.h"

#include <QSerialPort>
#include <QThread>

#include <algorithm>
#include <cstring>

/*  Legacy comutil.cpp transmitted one character at a time and slept
    10 ms after each one. The loom controllers were tuned against
    that pacing -- at 4800 baud it stretches a 15-byte Patronic
    command from ~31 ms to ~150 ms and guarantees a gap before the
    reply poll starts -- so it is reproduced here rather than
    writing whole blocks. */
static constexpr int SEND_CHAR_DELAY_MS = 10;

/*  GetChar() was non-blocking in legacy (InputCount() ? ReadChar()
    : '\0'). A 1 ms wait keeps that latency profile while still
    letting Qt service the port's overlapped I/O -- some legacy
    wait loops (StPatronicController::Terminate) never reach
    CheckAbort() and so never pump the event loop themselves. */
static constexpr int POLL_WAIT_MS = 1;

SerialPort::SerialPort()
{
    port = new QSerialPort();
}

SerialPort::~SerialPort()
{
    Close();
    delete port;
    port = nullptr;
}

/*-----------------------------------------------------------------*/
bool SerialPort::Open(const QString& _portName, const PORTINIT& _init)
{
    if (!port)
        return false;
    if (port->isOpen())
        return false;

    port->setPortName(_portName);

    QSerialPort::BaudRate br = QSerialPort::Baud9600;
    switch (_init.baudrate) {
    case BR_1200:
        br = QSerialPort::Baud1200;
        break;
    case BR_2400:
        br = QSerialPort::Baud2400;
        break;
    case BR_4800:
        br = QSerialPort::Baud4800;
        break;
    case BR_9600:
        br = QSerialPort::Baud9600;
        break;
    case BR_14400:
        br = QSerialPort::BaudRate(14400);
        break;
    }
    port->setBaudRate(br);

    port->setDataBits(_init.databits == DB_7 ? QSerialPort::Data7 : QSerialPort::Data8);

    QSerialPort::StopBits sb = QSerialPort::OneStop;
    switch (_init.stopbits) {
    case SB_ONE:
        sb = QSerialPort::OneStop;
        break;
    case SB_ONEFIVE:
        sb = QSerialPort::OneAndHalfStop;
        break;
    case SB_TWO:
        sb = QSerialPort::TwoStop;
        break;
    }
    port->setStopBits(sb);

    QSerialPort::Parity pa = QSerialPort::NoParity;
    switch (_init.parity) {
    case PA_NONE:
        pa = QSerialPort::NoParity;
        break;
    case PA_ODD:
        pa = QSerialPort::OddParity;
        break;
    case PA_EVEN:
        pa = QSerialPort::EvenParity;
        break;
    }
    port->setParity(pa);

    port->setFlowControl(QSerialPort::NoFlowControl);

    if (!port->open(QIODevice::ReadWrite)) {
        LoomLog::Write(QStringLiteral("open %1 FAILED: %2").arg(_portName, port->errorString()));
        return false;
    }

    /*  Legacy TComPort left DTR and RTS in their default (asserted)
        state. Qt's Windows backend unconditionally rewrites the DCB
        to RTS_CONTROL_DISABLE whenever flow control is not hardware
        handshaking, so RTS would sit low for the whole session --
        invisible on Linux/macOS (the kernel raises both lines on
        open) but fatal for any interface box that draws its enable
        level or loop current off RTS. Restore the legacy state; the
        user can turn it off from the loom options dialog if their
        box wants the lines low. */
    if (assertLines) {
        const bool dtr = port->setDataTerminalReady(true);
        const bool rts = port->setRequestToSend(true);
        LoomLog::Write(QStringLiteral("asserted DTR=%1 RTS=%2")
                           .arg(dtr ? QStringLiteral("ok") : QStringLiteral("refused"),
                                rts ? QStringLiteral("ok") : QStringLiteral("refused")));
    }

    LoomLog::Write(QStringLiteral("open %1 ok: %2 baud, %3 data, %4 parity, %5 stop, "
                                  "assert-lines=%6")
                       .arg(_portName)
                       .arg(int(br))
                       .arg(_init.databits == DB_7 ? 7 : 8)
                       .arg(_init.parity == PA_NONE   ? QStringLiteral("none")
                            : _init.parity == PA_EVEN ? QStringLiteral("even")
                                                      : QStringLiteral("odd"))
                       .arg(_init.stopbits == SB_ONE       ? QStringLiteral("1")
                            : _init.stopbits == SB_ONEFIVE ? QStringLiteral("1.5")
                                                           : QStringLiteral("2"))
                       .arg(assertLines ? 1 : 0));

    rxBuf.clear();
    return true;
}

bool SerialPort::IsOpen() const
{
    return port && port->isOpen();
}

void SerialPort::Close()
{
    if (port && port->isOpen()) {
        port->close();
        LoomLog::Write(QStringLiteral("closed"));
    }
    rxBuf.clear();
}

/*-----------------------------------------------------------------*/
bool SerialPort::Send(const char* _buffer)
{
    if (!_buffer)
        return false;
    return Send(_buffer, int(std::strlen(_buffer)));
}

bool SerialPort::Send(const char* _buffer, int _length)
{
    if (!port || !port->isOpen() || !_buffer)
        return false;
    if (_length <= 0)
        return true;

    LoomLog::WriteBytes(QStringLiteral("tx"), _buffer, _length);

    /*  One character at a time with a 10 ms gap -- see
        SEND_CHAR_DELAY_MS. waitForBytesWritten() forces each byte
        out to the driver rather than letting Qt coalesce them in
        its write buffer until the event loop next runs. */
    for (int i = 0; i < _length; i++) {
        if (port->write(_buffer + i, 1) != 1) {
            LoomLog::Write(
                QStringLiteral("tx FAILED at byte %1: %2").arg(i).arg(port->errorString()));
            return false;
        }
        port->waitForBytesWritten(2000);
        QThread::msleep(SEND_CHAR_DELAY_MS);
    }
    return true;
}

/*-----------------------------------------------------------------*/
bool SerialPort::Receive(char* _buffer, int _length)
{
    if (!_buffer || _length <= 0)
        return false;
    std::memset(_buffer, 0, _length);
    drainInto(200);
    const int n = std::min(_length - 1, int(rxBuf.size()));
    if (n > 0) {
        std::memcpy(_buffer, rxBuf.constData(), n);
        rxBuf.remove(0, n);
    }
    return true;
}

char SerialPort::GetChar()
{
    if (rxBuf.isEmpty())
        drainInto(POLL_WAIT_MS);
    if (rxBuf.isEmpty())
        return '\0';
    const char c = rxBuf.at(0);
    rxBuf.remove(0, 1);
    return c;
}

void SerialPort::PurgeInput()
{
    if (LoomLog::IsEnabled()) {
        /*  Anything still queued at purge time is data the protocol
            deliberately threw away; log it, because a reply landing
            just before a purge is one of the ways these handshakes
            deadlock. */
        if (!rxBuf.isEmpty())
            LoomLog::WriteBytes(QStringLiteral("purge"), rxBuf.constData(), int(rxBuf.size()));
    }
    rxBuf.clear();
    if (port && port->isOpen()) {
        port->clear(QSerialPort::Input);
    }
}

/*-----------------------------------------------------------------*/
void SerialPort::drainInto(int _waitMs)
{
    if (!port || !port->isOpen())
        return;
    /*  waitForReadyRead blocks until at least one byte is buffered
        by the OS (or the timeout fires). Kept short so this stays
        a poll rather than a blocking read. */
    if (port->bytesAvailable() == 0)
        port->waitForReadyRead(_waitMs);
    if (port->bytesAvailable() > 0) {
        const QByteArray chunk = port->readAll();
        LoomLog::WriteBytes(QStringLiteral("rx"), chunk.constData(), int(chunk.size()));
        rxBuf.append(chunk);
    }
}

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

#include <QSerialPort>
#include <QThread>

#if defined(Q_OS_WIN)
#define DBW_COM_PREFIX "COM"
#else
/*  Linux/macOS: /dev/ttyS* is the classic 16550-style; the user
    can edit to /dev/ttyUSB0 etc. before running. Real hardware on
    modern machines is almost always a USB adapter. */
#define DBW_COM_PREFIX "/dev/ttyS"
#endif

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
QString SerialPort::portName(PORT _p)
{
    int n = int(_p) - int(P_COM1);
    if (n < 0)
        n = 0;
#if defined(Q_OS_WIN)
    return QStringLiteral(DBW_COM_PREFIX "%1").arg(n + 1);
#else
    return QStringLiteral(DBW_COM_PREFIX "%1").arg(n);
#endif
}

/*-----------------------------------------------------------------*/
bool SerialPort::Open(PORT _port, const PORTINIT& _init)
{
    if (!port)
        return false;
    if (port->isOpen())
        return false;

    port->setPortName(portName(_port));

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

    if (!port->open(QIODevice::ReadWrite))
        return false;
    rxBuf.clear();
    return true;
}

bool SerialPort::IsOpen() const
{
    return port && port->isOpen();
}

void SerialPort::Close()
{
    if (port && port->isOpen())
        port->close();
    rxBuf.clear();
}

/*-----------------------------------------------------------------*/
bool SerialPort::Send(const char* _buffer)
{
    if (!port || !port->isOpen() || !_buffer)
        return false;
    const QByteArray data(_buffer);
    const qint64 written = port->write(data);
    if (written != data.size())
        return false;
    return port->waitForBytesWritten(2000);
}

bool SerialPort::Send(const char* _buffer, int _length)
{
    if (!port || !port->isOpen() || !_buffer)
        return false;
    const qint64 written = port->write(_buffer, _length);
    if (written != _length)
        return false;
    return port->waitForBytesWritten(2000);
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
        drainInto(50);
    if (rxBuf.isEmpty())
        return '\0';
    const char c = rxBuf.at(0);
    rxBuf.remove(0, 1);
    return c;
}

void SerialPort::PurgeInput()
{
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
        by the OS (or the timeout fires). */
    if (port->bytesAvailable() == 0)
        port->waitForReadyRead(_waitMs);
    if (port->bytesAvailable() > 0)
        rxBuf.append(port->readAll());
}

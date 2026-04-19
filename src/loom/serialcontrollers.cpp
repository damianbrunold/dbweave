/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "serialcontrollers.h"
#include "serialport.h"

#include <QDeadlineTimer>
#include <QThread>

#include <cstdio>
#include <cstring>
#include <memory>

/*-----------------------------------------------------------------*/
static constexpr int TIMEOUT_MS = 5000;

/*-----------------------------------------------------------------*/
StSerialController::StSerialController ()
{
	init_arm_patronic.parity   = PA_EVEN;
	init_arm_patronic.stopbits = SB_ONE;
	init_arm_patronic.baudrate = BR_4800;
	init_arm_patronic.databits = DB_7;

	init_arm_designer.parity   = PA_NONE;
	init_arm_designer.stopbits = SB_TWO;
	init_arm_designer.baudrate = BR_2400;
	init_arm_designer.databits = DB_8;

	init_slips.parity   = PA_NONE;
	init_slips.stopbits = SB_ONE;
	init_slips.baudrate = BR_9600;
	init_slips.databits = DB_8;

	init_avlcdiii.parity   = PA_NONE;
	init_avlcdiii.stopbits = SB_ONE;
	init_avlcdiii.baudrate = BR_9600;
	init_avlcdiii.databits = DB_8;

	serialport = new SerialPort();
}

StSerialController::~StSerialController ()
{
	delete serialport;
}

bool StSerialController::Initialize (const INITDATA& _data)
{
	return StWeaveController::Initialize(_data);
}

void StSerialController::Terminate ()
{
	if (serialport) serialport->Close();
}

/*-----------------------------------------------------------------*/
bool StPatronicController::Initialize (const INITDATA& _data)
{
	if (!StSerialController::Initialize(_data)) return false;
	if (!serialport) return false;
	if (!serialport->Open(PORT(_data.port), init_arm_patronic)) return false;

	serialport->PurgeInput();
	serialport->Send("M)D14.50K0AB\r");
	try {
		checkSuccess(true, true);
	} catch (...) {
		serialport->Close();
		return false;
	}
	return true;
}

bool StPatronicController::checkSuccess (bool _checkAbort, bool _timeout)
{
	char ch;
	QDeadlineTimer deadline(TIMEOUT_MS);
	do {
		if (_checkAbort) CheckAbort();
		ch = serialport->GetChar();
		if (_timeout && deadline.hasExpired()) throw 0;
	} while (ch != '\r');
	ch = serialport->GetChar();
	serialport->PurgeInput();
	return ch == '\r';
}

bool StPatronicController::waitForAck ()
{
	char ch;
	do {
		CheckAbort();
		ch = serialport->GetChar();
	} while (ch != 'Q' && ch != 'Z');
	serialport->PurgeInput();
	return ch == 'Q';
}

WEAVE_STATUS StPatronicController::WeaveSchuss (std::uint32_t _shafts)
{
	char buff[32];
	const unsigned s01_08 = (_shafts >> 16) & 0xff;
	const unsigned s09_16 = (_shafts >>  8) & 0xff;
	const unsigned s17_24 = _shafts          & 0xff;

	std::snprintf(buff, sizeof(buff), "M)D14.54K0%02X\r", s01_08);
	serialport->Send(buff);

	std::snprintf(buff, sizeof(buff), "M)D14.52K0%02X%02X\r", s09_16, s17_24);
	serialport->Send(buff);
	try {
		if (!checkSuccess(true, false)) return WEAVE_REPEAT;
	} catch (...) {
		return WEAVE_REPEAT;
	}

	try {
		return waitForAck() ? WEAVE_SUCCESS_NEXT : WEAVE_REPEAT;
	} catch (...) {
		return WEAVE_REPEAT;
	}
}

void StPatronicController::Terminate ()
{
	if (serialport && serialport->IsOpen()) {
		serialport->Send("M)D14.50K0\r");
		try { checkSuccess(false, false); } catch (...) {}
		serialport->Close();
	}
}

/*-----------------------------------------------------------------*/
bool StPatronicIndirectController::Initialize (const INITDATA& _data)
{
	if (!StSerialController::Initialize(_data)) return false;
	if (!serialport) return false;
	return serialport->Open(PORT(_data.port), init_arm_patronic);
}

bool StPatronicIndirectController::checkSuccess ()
{
	char ch;
	QDeadlineTimer deadline(TIMEOUT_MS);
	do {
		CheckAbort();
		ch = serialport->GetChar();
		if (deadline.hasExpired()) { serialport->Close(); throw 0; }
	} while (ch != '\r');
	ch = serialport->GetChar();
	serialport->PurgeInput();
	return ch == '\r';
}

bool StPatronicIndirectController::waitForAck ()
{
	char ch;
	do {
		CheckAbort();
		ch = serialport->GetChar();
	} while (ch != 'R');
	serialport->PurgeInput();
	return ch == 'R';
}

WEAVE_STATUS StPatronicIndirectController::WeaveSchuss (std::uint32_t _shafts)
{
	if (position > 220) return WEAVE_SUCCESS_NEXT;

	char buff[32];
	const unsigned s01_08 = (_shafts >> 16) & 0xff;
	const unsigned s09_16 = (_shafts >>  8) & 0xff;
	const unsigned s17_24 = _shafts          & 0xff;

	char pos1[8];
	char pos2[8];
	const int high = position / 32;
	const int low  = position % 32;
	std::snprintf(pos1, sizeof(pos1), "%02d.%02d", high,     low * 2);
	std::snprintf(pos2, sizeof(pos2), "%02d.%02d", high + 7, low * 2);

	std::snprintf(buff, sizeof(buff), "M)D%sK0%02X%02X\r", pos1, s09_16, s17_24);
	serialport->Send(buff);
	std::snprintf(buff, sizeof(buff), "M)D%sK000%02X\r",   pos2, s01_08);
	serialport->Send(buff);

	try {
		if (!checkSuccess()) return WEAVE_REPEAT;
	} catch (...) {
		return WEAVE_REPEAT;
	}
	try {
		const bool ok = waitForAck();
		if (ok) position++;
		return ok ? WEAVE_SUCCESS_NEXT : WEAVE_REPEAT;
	} catch (...) {
		return WEAVE_REPEAT;
	}
}

void StPatronicIndirectController::Terminate ()
{
	if (serialport && serialport->IsOpen()) serialport->Close();
}

/*-----------------------------------------------------------------*/
bool StDesignerController::Initialize (const INITDATA& _data)
{
	if (!StSerialController::Initialize(_data)) return false;
	if (!serialport) return false;
	return serialport->Open(PORT(_data.port), init_arm_designer);
}

void StDesignerController::waitFachGeschlossen ()
{
	QThread::msleep(50);
	char ch;
	QDeadlineTimer deadline(TIMEOUT_MS);
	do {
		CheckAbort();
		ch = serialport->GetChar();
		if (deadline.hasExpired()) throw 0;
	} while (ch != '1');
}

void StDesignerController::waitFachOffen ()
{
	QThread::msleep(50);
	char ch;
	QDeadlineTimer deadline(TIMEOUT_MS);
	do {
		CheckAbort();
		ch = serialport->GetChar();
		if (deadline.hasExpired()) throw 0;
	} while (ch != '0');
}

void StDesignerController::sendMuster (std::uint32_t _shafts)
{
	char buffer[4];
	buffer[0] = 2;   /* STX */
	buffer[1] = char( _shafts        & 0xff);
	buffer[2] = char((_shafts >>  8) & 0xff);
	buffer[3] = char((_shafts >> 16) & 0xff);
	serialport->Send(buffer, 4);
}

WEAVE_STATUS StDesignerController::WeaveSchuss (std::uint32_t _shafts)
{
	try {
		waitFachGeschlossen();
		sendMuster(_shafts);
		waitFachOffen();
		waitFachGeschlossen();
	} catch (...) {
		return WEAVE_REPEAT;
	}
	return WEAVE_SUCCESS_NEXT;
}

/*-----------------------------------------------------------------*/
bool StSlipsController::Initialize (const INITDATA& _data)
{
	if (!StSerialController::Initialize(_data)) return false;
	if (!serialport) return false;
	forward = true;
	return serialport->Open(PORT(_data.port), init_slips);
}

void StSlipsController::SetBytes (int _bytes)
{
	if (_bytes < 0) _bytes = 0;
	if (_bytes > 4) _bytes = 4;
	bytes = _bytes;
}

void StSlipsController::getData (int& _gear, int& _lift)
{
	bool changed = false;
	int gear = _gear;
	int lift = _lift;
	char ch;
	do {
		CheckAbort();
		ch   = serialport->GetChar();
		gear = int((ch & 2) >> 1);
		lift = int( ch & 1);
		changed = (gear != _gear || lift != _lift);
	} while (!changed);
	_gear = gear;
	_lift = lift;
	serialport->PurgeInput();
}

bool StSlipsController::sendData (std::uint32_t _shafts)
{
	char buff[5];
	buff[0] = char( _shafts        & 0xff);
	buff[1] = char((_shafts >>  8) & 0xff);
	buff[2] = char((_shafts >> 16) & 0xff);
	buff[3] = char((_shafts >> 24) & 0xff);
	buff[4] = 0;
	serialport->Send(buff, bytes);
	return true;
}

WEAVE_STATUS StSlipsController::WeaveSchuss (std::uint32_t _shafts)
{
	/*  Mirrors the legacy SLIPS state machine over the gear/lift
	    pair: H=0, L=1. */
	enum { H = 0, L = 1 };
	int state = 1;
	int gear  = H;
	int lift  = H;
	bool repeat = false;

	try {
		while (state != 0) {
			getData(gear, lift);
			switch (state) {
			case 1:
				if (gear == H && lift == L) { sendData(_shafts); state = 2; break; }
				if (gear == L && lift == H) { state = 4; break; }
				break;
			case 2:
				if (gear == L && lift == H) { state = 3; break; }
				if (gear == H && lift == H) { state = 0; break; }
				break;
			case 3:
				if (gear == L && lift == L) { repeat = true; state = 0; break; }
				if (gear == H && lift == H) { state = 0; break; }
				break;
			case 4:
				if (gear == H && lift == H) { forward = !forward; state = 0; break; }
				if (gear == L && lift == L) { repeat  = true;     state = 0; break; }
				break;
			}
		}
	} catch (...) {
		return WEAVE_REPEAT;
	}

	if (repeat) return forward ? WEAVE_SUCCESS_PREV : WEAVE_SUCCESS_NEXT;
	return forward ? WEAVE_SUCCESS_NEXT : WEAVE_SUCCESS_PREV;
}

void StSlipsController::Terminate ()
{
	if (serialport && serialport->IsOpen()) serialport->Close();
}

/*-----------------------------------------------------------------*/
bool StAvlCdIIIController::Initialize (const INITDATA& _data)
{
	if (!StSerialController::Initialize(_data)) return false;
	if (!serialport) return false;
	if (!serialport->Open(PORT(_data.port), init_avlcdiii)) return false;

	serialport->PurgeInput();
	char buff[3] = { 0x0f, 0x07, 0 };
	serialport->Send(buff, 2);
	try {
		matchReply(0x7f, 0x03, true);
	} catch (...) {
		serialport->Close();
		return false;
	}
	serialport->PurgeInput();
	return true;
}

void StAvlCdIIIController::matchReply (int _r, bool _timeout)
{
	QDeadlineTimer deadline(TIMEOUT_MS);
	while (true) {
		CheckAbort();
		const int r = static_cast<unsigned char>(serialport->GetChar());
		if (r == _r) break;
		if (_timeout && deadline.hasExpired()) throw 0;
	}
}

void StAvlCdIIIController::matchReply (int _r1, int _r2, bool _timeout)
{
	matchReply(_r1, _timeout);
	matchReply(_r2, _timeout);
	serialport->PurgeInput();
}

bool StAvlCdIIIController::waitForUp ()
{
	try { matchReply(0x61, 0x03, false); return true; } catch (...) { return false; }
}

bool StAvlCdIIIController::waitForDown ()
{
	try { matchReply(0x62, 0x03, false); return true; } catch (...) { return false; }
}

WEAVE_STATUS StAvlCdIIIController::WeaveSchuss (std::uint32_t _shafts)
{
	serialport->PurgeInput();

	char buff[10];
	buff[0] = char(0x10 | ( _shafts        & 0xf));
	buff[1] = char(0x20 | ((_shafts >>  4) & 0xf));
	buff[2] = char(0x30 | ((_shafts >>  8) & 0xf));
	buff[3] = char(0x40 | ((_shafts >> 12) & 0xf));
	buff[4] = char(0x50 | ((_shafts >> 16) & 0xf));
	buff[5] = char(0x60 | ((_shafts >> 20) & 0xf));
	buff[6] = char(0x70 | ((_shafts >> 24) & 0xf));
	buff[7] = char(0x80 | ((_shafts >> 28) & 0xf));
	buff[8] = 0x07;
	buff[9] = 0;

	serialport->Send(buff, 9);
	waitForUp();
	waitForDown();
	return WEAVE_SUCCESS_NEXT;
}

void StAvlCdIIIController::Terminate ()
{
	serialport->PurgeInput();
	if (serialport && serialport->IsOpen()) {
		char buff[3] = { 0x0f, 0x07, 0 };
		serialport->Send(buff, 2);
		try { matchReply(0x7f, 0x03, true); } catch (...) {}
		serialport->Close();
	}
}

/*-----------------------------------------------------------------*/
std::unique_ptr<StWeaveController> makeLoomController (LOOMINTERFACE _intrf)
{
	switch (_intrf) {
	case intrf_arm_patronic:          return std::make_unique<StPatronicController>();
	case intrf_arm_patronic_indirect: return std::make_unique<StPatronicIndirectController>();
	case intrf_arm_designer:          return std::make_unique<StDesignerController>();
	case intrf_slips:                 return std::make_unique<StSlipsController>();
	case intrf_avl_cd_iii:            return std::make_unique<StAvlCdIIIController>();
	default:                          return std::make_unique<StDummyController>();
	}
}

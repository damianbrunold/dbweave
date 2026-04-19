/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Loom-control abstraction (Phase 11). Simplified port of legacy
    steuerung.h / comutil.h that keeps only the dummy-loom path:
    every WeaveSchuss call reports success after a short wait.

    Real serial controllers (ARM Patronic, ARM Designer, Slips,
    Varpapuu Parallel, AVL CD-III) are deliberately not ported
    here — they need QSerialPort plus protocol-specific byte
    sequences that differ per loom model. The dummy controller is
    enough to drive the simulator in LoomDialog; when real
    hardware becomes relevant the serial subclasses drop in as
    siblings of StDummyController.                               */

#ifndef DBWEAVE_UI_LOOM_H
#define DBWEAVE_UI_LOOM_H

#include <cstdint>

/*  Which loom the user selected — kept as an enum for future
    serial-controller expansion. Only intrf_dummy has a live
    controller implementation in the port. */
enum LOOMINTERFACE {
	intrf_dummy = 0,
	intrf_arm_patronic,
	intrf_arm_patronic_indirect,
	intrf_arm_designer,
	intrf_varpapuu_parallel,
	intrf_slips,
	intrf_lips,
	intrf_avl_cd_iii
};

enum WEAVE_STATUS {
	WEAVE_REPEAT,
	WEAVE_SUCCESS_NEXT,
	WEAVE_SUCCESS_PREV
};

struct INITDATA
{
	int port  = 0;
	int lpt   = 0;
	int delay = 0;
	int port1 = 0;
	int port2 = 0;
};

/*-----------------------------------------------------------------*/
class StWeaveController
{
public:
	StWeaveController ();
	virtual ~StWeaveController ();

	/*  Sets `aborted=false` and returns success. Subclasses open
	    their physical port here. */
	virtual bool Initialize (const INITDATA& _data);

	/*  Sends a single weft row (bit-per-shaft). Returns
	    WEAVE_SUCCESS_NEXT to advance, WEAVE_SUCCESS_PREV to step
	    back, or WEAVE_REPEAT to retry the same row. */
	virtual WEAVE_STATUS WeaveSchuss (std::uint32_t _shafts) = 0;

	/*  Loom-specific parameter nudged by the UI (e.g. patronic-
	    indirect's memory offset). Ignored by dummy. */
	virtual void SetSpecialData (int _data) { (void)_data; }

	/*  Closes / resets the loom. */
	virtual void Terminate ();

	/*  Cooperative abort: a Stop click sets `aborted=true`; the
	    next CheckAbort inside WeaveSchuss throws to exit the
	    loop. */
	void Abort ();
	bool IsAborted () const { return aborted; }

protected:
	bool aborted = false;

	/*  Subclasses call this inside their wait loops to give the
	    Qt event loop a slice and bail out if Abort fired. */
	void CheckAbort ();
};

/*-----------------------------------------------------------------*/
class StDummyController : public StWeaveController
{
public:
	StDummyController () = default;

	bool Initialize (const INITDATA& _data) override;
	WEAVE_STATUS WeaveSchuss (std::uint32_t _shafts) override;
	void Terminate () override;

	/*  Tunable wait time per schuss (milliseconds). Defaults to
	    the legacy 1000 ms. LoomDialog exposes this as a UI knob
	    so the user can sim through a pattern quickly. */
	void setWaitMs (int _ms) { waitMs = _ms; }

private:
	int waitMs = 1000;
};

#endif

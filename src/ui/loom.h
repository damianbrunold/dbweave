/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Loom-control abstraction. StWeaveController + StDummyController
    live here unconditionally; the four real serial controllers
    (ARM Patronic direct / indirect, ARM Designer, Generic SLIPS,
    AVL CD-III) land as siblings in src/loom/serialcontrollers.cpp
    when the loom module is compiled in. makeLoomController(...)
    returns the concrete controller matching a LOOMINTERFACE.     */

#ifndef DBWEAVE_UI_LOOM_H
#define DBWEAVE_UI_LOOM_H

#include <cstdint>

/*  Which loom the user selected. The two parallel-port interfaces
    (Varpapuu Parallel at slot 4, generic LIPS at slot 6) were
    listed in the legacy UI but never implemented by any controller
    -- direct parallel-port I/O isn't available on NT+ without a
    kernel driver, so the entries always fell through to the dummy.
    The enum slots are reserved (skipped) to keep the integer
    values stable with saved QSettings "Loom/Interface" keys.    */
enum LOOMINTERFACE {
    intrf_dummy = 0,
    intrf_arm_patronic = 1,
    intrf_arm_patronic_indirect = 2,
    intrf_arm_designer = 3,
    /* 4 -- was intrf_varpapuu_parallel, never implemented */
    intrf_slips = 5,
    /* 6 -- was intrf_lips, never implemented */
    intrf_avl_cd_iii = 7
};

enum WEAVE_STATUS { WEAVE_REPEAT, WEAVE_SUCCESS_NEXT, WEAVE_SUCCESS_PREV };

struct INITDATA {
    int port = 0;
};

/*-----------------------------------------------------------------*/
class StWeaveController
{
public:
    StWeaveController();
    virtual ~StWeaveController();

    /*  Sets `aborted=false` and returns success. Subclasses open
        their physical port here. */
    virtual bool Initialize(const INITDATA& _data);

    /*  Sends a single weft row (bit-per-shaft). Returns
        WEAVE_SUCCESS_NEXT to advance, WEAVE_SUCCESS_PREV to step
        back, or WEAVE_REPEAT to retry the same row. */
    virtual WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) = 0;

    /*  Loom-specific parameter nudged by the UI (e.g. patronic-
        indirect's memory offset). Ignored by dummy. */
    virtual void SetSpecialData(int _data)
    {
        (void)_data;
    }

    /*  Closes / resets the loom. */
    virtual void Terminate();

    /*  Cooperative abort: a Stop click sets `aborted=true`; the
        next CheckAbort inside WeaveSchuss throws to exit the
        loop. */
    void Abort();
    bool IsAborted() const
    {
        return aborted;
    }

protected:
    bool aborted = false;

    /*  Subclasses call this inside their wait loops to give the
        Qt event loop a slice and bail out if Abort fired. */
    void CheckAbort();
};

/*-----------------------------------------------------------------*/
class StDummyController : public StWeaveController
{
public:
    StDummyController() = default;

    bool Initialize(const INITDATA& _data) override;
    WEAVE_STATUS WeaveSchuss(std::uint32_t _shafts) override;
    void Terminate() override;

    /*  Tunable wait time per schuss (milliseconds). Defaults to
        the legacy 1000 ms. LoomDialog exposes this as a UI knob
        so the user can sim through a pattern quickly. */
    void setWaitMs(int _ms)
    {
        waitMs = _ms;
    }

private:
    int waitMs = 1000;
};

#endif

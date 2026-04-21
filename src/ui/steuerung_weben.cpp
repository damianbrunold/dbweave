/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Port of legacy steuerung_weben.cpp. Runs the weaving loop in
    the UI thread, pumping the Qt event loop between schuss calls
    so the Stop button and status-bar updates remain responsive. */

#include "steuerung.h"

#include "datamodule.h"
#include "language.h"
#include "steuerungcanvas.h"

#include <QAction>
#include <QApplication>
#include <QMessageBox>

#include <algorithm>

#ifdef DBWEAVE_HAVE_LOOM
#include "serialcontrollers.h"
#endif

/*-----------------------------------------------------------------*/
/*  Build a controller matching the current `intrf` selection.
    Without DBWEAVE_HAVE_LOOM, every non-dummy selection falls back
    to the dummy simulator -- the UI still lets the user step the
    pattern even though no real serial hardware is reachable.      */
void TSTRGFRM::AllocInterface()
{
    if (controller) {
        controller->Terminate();
        controller.reset();
    }

#ifdef DBWEAVE_HAVE_LOOM
    controller = makeLoomController(intrf);
    /*  SLIPS controller has a bytes-per-schuss knob keyed off the
        shaft count. Downcast once here; no-op for other controllers. */
    if (intrf == intrf_slips) {
        if (auto* slips = dynamic_cast<StSlipsController*>(controller.get())) {
            int bytes = 4;
            if (numberOfShafts <= 8)
                bytes = 1;
            else if (numberOfShafts <= 16)
                bytes = 2;
            else if (numberOfShafts <= 24)
                bytes = 3;
            slips->SetBytes(bytes);
        }
    }
#else
    controller = std::make_unique<StDummyController>();
#endif

    /*  In SLIPS mode the loom drives direction; hide the manual
        reverse toggle the same way legacy did.                    */
    if (actReverse)
        actReverse->setVisible(intrf != intrf_slips && intrf != intrf_lips);
}

/*-----------------------------------------------------------------*/
int TSTRGFRM::MaxSchaefte() const
{
    return numberOfShafts;
}

/*-----------------------------------------------------------------*/
/*  Returns true when the weaver's cursor is at the beginning of
    the first non-empty klammer. Used by the Weben loop to detect
    a completed single pass when Loop is unchecked.                */
bool TSTRGFRM::AtBegin() const
{
    int klammer;
    int position = 0;
    int repetition = 1;
    for (klammer = 0; klammer < MAXKLAMMERN; klammer++) {
        if (klammern[klammer].repetitions > 0) {
            position = klammern[klammer].first;
            repetition = 1;
            break;
        }
    }
    return weave_klammer == klammer && weave_position == position
        && weave_repetition == repetition;
}

/*-----------------------------------------------------------------*/
/*  Build the 24- (or MaxSchaefte-) bit shaft mask for the current
    `weave_position`. Mirrors legacy GetSchaftDaten; each loom
    family gets its own bit-order:
      - arm_patronic / arm_patronic_indirect:  MSB = shaft 1.
      - arm_designer:                          MSB = shaft (max-1).
      - varpapuu_parallel:                     LSB = shaft 1.
      - default (SLIPS / LIPS / AVL / Dummy):  MSB = shaft (max-1).
    `reverse` flips the ordering within each family.              */
std::uint32_t TSTRGFRM::GetSchaftDaten(int /*_pos*/) const
{
    if (!trittfolge || !aufknuepfung)
        return 0;
    std::uint32_t dataBits = 0;

    if (intrf == intrf_arm_patronic || intrf == intrf_arm_patronic_indirect) {
        const int mi = std::min(trittfolge->feld.SizeX(), 24);
        const int mj = std::min(mi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (int i = 0; i < mi; i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0) {
                    dataBits |= (1u << (reverse ? i : (23 - i)));
                }
            }
        } else {
            for (int i = 0; i < trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0) {
                    for (int k = 0; k < mj; k++) {
                        if (aufknuepfung->feld.Get(i, k) > 0)
                            dataBits |= (1u << (reverse ? k : (23 - k)));
                    }
                }
            }
        }
    } else if (intrf == intrf_arm_designer) {
        const int mi = std::min(trittfolge->feld.SizeX(), MaxSchaefte());
        const int mj = std::min(mi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (int i = 0; i < mi; i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0)
                    dataBits |= (1u << (reverse ? i : (mi - 1 - i)));
            }
        } else {
            for (int i = 0; i < trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0) {
                    for (int k = 0; k < mj; k++) {
                        if (aufknuepfung->feld.Get(i, k) > 0)
                            dataBits |= (1u << (reverse ? k : (mi - 1 - k)));
                    }
                }
            }
        }
    } else if (intrf == intrf_varpapuu_parallel) {
        const int mi = std::min(trittfolge->feld.SizeX(), MaxSchaefte());
        const int mj = std::min(mi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (int i = 0; i < mi; i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0)
                    dataBits |= (1u << (reverse ? (mi - 1 - i) : i));
            }
        } else {
            for (int i = 0; i < trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0) {
                    for (int k = 0; k < mj; k++) {
                        if (aufknuepfung->feld.Get(i, k) > 0)
                            dataBits |= (1u << (reverse ? (mi - 1 - k) : k));
                    }
                }
            }
        }
    } else {
        const int mi = std::min(trittfolge->feld.SizeX(), MaxSchaefte());
        const int mj = std::min(mi, aufknuepfung->feld.SizeY());
        if (schlagpatrone) {
            for (int i = 0; i < mi; i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0)
                    dataBits |= (1u << (reverse ? i : (mi - 1 - i)));
            }
        } else {
            for (int i = 0; i < trittfolge->feld.SizeX(); i++) {
                if (trittfolge->feld.Get(i, weave_position) > 0) {
                    for (int k = 0; k < mj; k++) {
                        if (aufknuepfung->feld.Get(i, k) > 0)
                            dataBits |= (1u << (reverse ? k : (mi - 1 - k)));
                    }
                }
            }
        }
    }
    return dataBits;
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::NextTritt()
{
    weave_position++;
    if (weave_position > klammern[weave_klammer].last) {
        if (weave_repetition < klammern[weave_klammer].repetitions) {
            weave_repetition++;
            weave_position = klammern[weave_klammer].first;
        } else {
            weave_klammer = (weave_klammer + 1) % MAXKLAMMERN;
            while (klammern[weave_klammer].repetitions == 0)
                weave_klammer = (weave_klammer + 1) % MAXKLAMMERN;
            weave_repetition = 1;
            weave_position = klammern[weave_klammer].first;
        }
    }
    current_klammer = weave_klammer;
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::PrevTritt()
{
    weave_position--;
    if (weave_position < klammern[weave_klammer].first) {
        if (weave_repetition > 1) {
            weave_repetition--;
            weave_position = klammern[weave_klammer].last;
        } else {
            weave_klammer = (weave_klammer + MAXKLAMMERN - 1) % MAXKLAMMERN;
            while (klammern[weave_klammer].repetitions == 0)
                weave_klammer = (weave_klammer + MAXKLAMMERN - 1) % MAXKLAMMERN;
            weave_repetition = klammern[weave_klammer].repetitions;
            weave_position = klammern[weave_klammer].last;
        }
    }
    current_klammer = weave_klammer;
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::WeaveStartClick()
{
    if (!IsValidWeavePosition()) {
        const int r = QMessageBox::question(this,
                                            LANG_STR("DB-WEAVE", "DB-WEAVE"),
                                            LANG_STR("The current position is not valid for "
                                                     "any active klammer. Reset and continue?",
                                                     "Die aktuelle Position ist in keiner "
                                                     "aktiven Klammer gültig. Zurücksetzen und "
                                                     "fortfahren?"));
        if (r != QMessageBox::Yes)
            return;
        _ResetCurrentPos();
        UpdateStatusbar();
    }
    if (klammern[weave_klammer].repetitions == 0) {
        QMessageBox::information(
            this, LANG_STR("DB-WEAVE", "DB-WEAVE"),
            LANG_STR("No klammer configured -- nothing to weave.",
                     "Keine Klammer konfiguriert -- nichts zu weben."));
        return;
    }
    if (!controller) {
        QMessageBox::warning(this, LANG_STR("DB-WEAVE", "DB-WEAVE"),
                             LANG_STR("Could not initialise the loom controller.",
                                      "Der Webstuhl-Controller konnte nicht initialisiert werden."));
        return;
    }
    INITDATA init;
    init.port = port;
    init.lpt = lpt;
    init.delay = delay;
    if (!controller->Initialize(init)) {
        QMessageBox::warning(this, LANG_STR("DB-WEAVE", "DB-WEAVE"),
                             LANG_STR("Could not talk to the loom.",
                                      "Kommunikation mit dem Webstuhl fehlgeschlagen."));
        return;
    }

    stopit = false;
    tempquit = false;
    weaving = true;
    firstschuss = true;

    if (actStart)
        actStart->setEnabled(false);
    if (actStop)
        actStop->setEnabled(true);
    if (actOptionsLoom)
        actOptionsLoom->setEnabled(false);

    Weben();

    /*  Teardown: legacy always calls controller->Terminate here. */
    try {
        if (controller)
            controller->Terminate();
    } catch (...) {
    }
    stopit = true;

    if (actStart)
        actStart->setEnabled(true);
    if (actStop)
        actStop->setEnabled(false);
    if (actOptionsLoom)
        actOptionsLoom->setEnabled(true);

    UpdateStatusbar();
    if (canvas)
        canvas->update();
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::WeaveStopClick()
{
    if (controller && !controller->IsAborted()) {
        controller->Abort();
        stopit = true;
        weaving = false;
        tempquit = false;
    }
}

/*-----------------------------------------------------------------*/
void TSTRGFRM::WeaveTempQuit()
{
    if (controller && !controller->IsAborted()) {
        controller->Abort();
        stopit = true;
        weaving = true;
        tempquit = true;
    }
}

/*-----------------------------------------------------------------*/
/*  Main weaving loop. Pumps Qt events between schuss calls so the
    Stop action and the canvas repaint remain responsive. Legacy
    uses Application->ProcessMessages() here; Qt exposes the same
    via QCoreApplication::processEvents.                          */
void TSTRGFRM::Weben()
{
    if (!trittfolge || !aufknuepfung || !controller)
        return;
    int count = 0;
    while (!stopit) {
        const std::uint32_t shaftBits = GetSchaftDaten(weave_position);
        WEAVE_STATUS stat = WEAVE_REPEAT;
        bool success = false;
        UpdateLastPosition();
        if (canvas)
            canvas->update();
        try {
            stat = controller->WeaveSchuss(shaftBits);
            success = stat == WEAVE_SUCCESS_NEXT || stat == WEAVE_SUCCESS_PREV;
        } catch (...) {
        }
        QCoreApplication::processEvents();
        if (stopit)
            break;
        if (!success)
            continue;

        count++;
        if (maxweave != 0 && count >= maxweave) {
            QMessageBox::information(
                this, LANG_STR("DB-WEAVE", "DB-WEAVE"),
                LANG_STR("All picks sent.", "Alle Schüsse gesendet."));
            weaving = false;
            return;
        }
        if (weave_position == last_position) {
            const bool loomPicksDirection
                = (intrf == intrf_varpapuu_parallel || intrf == intrf_slips
                   || intrf == intrf_lips);
            if (loomPicksDirection) {
                if (stat == WEAVE_SUCCESS_NEXT)
                    NextTritt();
                else
                    PrevTritt();
            } else {
                if (!backwards)
                    NextTritt();
                else
                    PrevTritt();
            }
        }
        firstschuss = false;
        AutoScroll();
        UpdateStatusbar();
        if (canvas)
            canvas->update();
        if (!loop && !firstschuss && AtBegin()) {
            QApplication::beep();
            weaving = false;
            return;
        }
    }
}

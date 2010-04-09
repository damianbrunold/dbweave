// DB-WEAVE, textile CAD/CAM software
// Copyright (c) 1998-2005 Brunold Software, Switzerland

#include <vcl.h>
#pragma hdrstop

#include "dbw3_form.h"
#include "dbw3_strings.h"
#include "printselection_form.h"
#include "print.h"
#include "datamodule.h"
#include "language.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

SZ __fastcall CalcSchaefte()
{
    SZ schaefte;
	schaefte.a = -1;
    schaefte.b = -1;

	// ersten Schaft bestimmen
	for (int j=0; j<Data->MAXY1; j++) {
        if (!DBWFRM->freieschaefte[j]) {
            schaefte.a = j;
            break;
        }
	}

	// letzten Schaft bestimmen
	for (int j=Data->MAXY1-1; j>=0; j--) {
        if (!DBWFRM->freieschaefte[j]) {
            schaefte.b = j;
            break;
        }
	}

    return schaefte;
}

SZ __fastcall CalcTritte()
{
    SZ tritte;
	tritte.a = -1;
    tritte.b = -1;

	// ersten Schaft bestimmen
	for (int i=0; i<Data->MAXX2; i++) {
        if (!DBWFRM->freietritte[i]) {
            tritte.a = i;
            break;
        }
	}

	// letzten Schaft bestimmen
	for (int i=Data->MAXX2-1; i>=0; i--) {
        if (!DBWFRM->freietritte[i]) {
            tritte.b = i;
            break;
        }
	}

    return tritte;
}

void __fastcall TDBWFRM::OptPrintSelectionClick(TObject *Sender)
{
    try {
        TPrintSelectionForm* pFrm = new TPrintSelectionForm (this);
        pFrm->maxkette = gewebe.SizeX();
        pFrm->maxschuesse = gewebe.SizeY();
        pFrm->maxschaefte = einzug.SizeY();
        pFrm->maxtritte = trittfolge.SizeX();

        // Ganzes Muster als Voreinstellung
        if (!ViewEinzug->Checked) {
            pFrm->schaefte.a = -1;
            pFrm->schaefte.b = -1;
        } else {
            pFrm->schaefte = CalcSchaefte();
        }
        if (!ViewTrittfolge->Checked) {
            pFrm->tritte.a = -1;
            pFrm->tritte.b = -1;
        } else {
            pFrm->tritte = CalcTritte();
        }
        if (kette.b!=-1) pFrm->kette = kette;
        else {
            pFrm->kette.a = 0;
            pFrm->kette.b = 19;
        }
        if (schuesse.b!=-1) pFrm->schuesse = schuesse;
        else {
            pFrm->schuesse.a = 0;
            pFrm->schuesse.b = 19;
        }

        if (pFrm->ShowModal()==IDOK) {
            // Bereich drucken
            TCursor oldcursor = Cursor;
            Cursor = crHourGlass;
            PrPrinterPrint printer(this, Data);
            printer.PrintRange (pFrm->kette, pFrm->schuesse,
                                pFrm->schaefte, pFrm->tritte);
            Cursor = oldcursor;
        }
    } catch (...) {
    }
}

__fastcall TPrintSelectionForm::TPrintSelectionForm(TComponent* Owner)
: TForm(Owner)
{
    LoadLanguage();
}

SZ __fastcall TPrintSelectionForm::GetRange (TEdit* _von, TEdit* _bis)
{
    SZ range;
    try {
        range.a = StrToInt(_von->Text) - 1;
        range.b = StrToInt(_bis->Text) - 1;
    } catch (EConvertError& e) {
        range.a = 0;
        range.b = -1;
    }
    return range;
}

bool __fastcall TPrintSelectionForm::CheckRange(const SZ& _range, int _max)
{
    if (_range.a==_range.b && _range.a==-1) return true;
    return (_range.a>=0 && _range.b>=_range.a && _range.b<=_max);
}

void __fastcall TPrintSelectionForm::bOkClick(TObject *Sender)
{
    // Werte auslesen
    kette = GetRange (KetteVon, KetteBis);
    schuesse = GetRange (SchussVon, SchussBis);
    schaefte = GetRange (SchaftVon, SchaftBis);
    tritte = GetRange (TrittVon, TrittBis);

    if (!CheckRange (kette, maxkette)) {
        Application->MessageBox (INVALID_KETTRANGE, APP_TITLE, MB_OK);
        ActiveControl = KetteVon;
        ModalResult = mrNone;
        return;
    }

    if (!CheckRange (schuesse, maxschuesse)) {
        Application->MessageBox (INVALID_SCHUSSRANGE, APP_TITLE, MB_OK);
        ActiveControl = SchussVon;
        ModalResult = mrNone;
        return;
    }

    if (!CheckRange (schaefte, maxschaefte)) {
        Application->MessageBox (INVALID_SCHAFTRANGE, APP_TITLE, MB_OK);
        ActiveControl = SchaftVon;
        ModalResult = mrNone;
        return;
    }

    if (!CheckRange (tritte, maxtritte)) {
        Application->MessageBox (INVALID_TRITTRANGE, APP_TITLE, MB_OK);
        ActiveControl = TrittVon;
        ModalResult = mrNone;
        return;
    }

    ModalResult = mrOk;
}

void __fastcall TPrintSelectionForm::FormShow(TObject *Sender)
{
    KetteVon->Text = IntToStr(kette.a+1);
    KetteBis->Text = IntToStr(kette.b+1);

    SchussVon->Text = IntToStr(schuesse.a+1);
    SchussBis->Text = IntToStr(schuesse.b+1);

    SchaftVon->Text = IntToStr(schaefte.a+1);
    SchaftBis->Text = IntToStr(schaefte.b+1);

    TrittVon->Text = IntToStr(tritte.a+1);
    TrittBis->Text = IntToStr(tritte.b+1);
}

void __fastcall TPrintSelectionForm::LoadLanguage()
{
    LANG_C_H(this, EN, "Print part", "")
    LANG_C_H(this, GE, "Teil drucken", "")
    LANG_C_H(bOk, EN, "OK", "")
    LANG_C_H(bOk, GE, "OK", "")
    LANG_C_H(bCancel, EN, "Cancel", "")
    LANG_C_H(bCancel, GE, "Abbrechen", "")
    LANG_C_H(gbKette, EN, "&Warp range", "")
    LANG_C_H(gbKette, GE, "&Kettfadenbereich", "")
    LANG_C_H(lKetteVon, EN, "From", "")
    LANG_C_H(lKetteVon, GE, "Von", "")
    LANG_C_H(lKetteBis, EN, "To", "")
    LANG_C_H(lKetteBis, GE, "Bis", "")
    LANG_C_H(gbSchuesse, EN, "We&ft range", "")
    LANG_C_H(gbSchuesse, GE, "&Schussfadenbereich", "")
    LANG_C_H(lSchussVon, EN, "From", "")
    LANG_C_H(lSchussVon, GE, "Von", "")
    LANG_C_H(lSchussBis, EN, "To", "")
    LANG_C_H(lSchussBis, GE, "Bis", "")
    LANG_C_H(gbSchaefte, EN, "&Harness range", "")
    LANG_C_H(gbSchaefte, GE, "S&chaftbereich", "")
    LANG_C_H(lSchaftVon, EN, "From", "")
    LANG_C_H(lSchaftVon, GE, "Von", "")
    LANG_C_H(lSchaftBis, EN, "To", "")
    LANG_C_H(lSchaftBis, GE, "Bis", "")
    LANG_C_H(gbTritte, EN, "&Treadle range", "")
    LANG_C_H(gbTritte, GE, "&Trittbereich", "")
    LANG_C_H(lTrittVon, EN, "From", "")
    LANG_C_H(lTrittVon, GE, "Von", "")
    LANG_C_H(lTrittBis, EN, "To", "")
    LANG_C_H(lTrittBis, GE, "Bis", "")
}


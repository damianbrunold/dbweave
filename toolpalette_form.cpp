/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File toolpalette_form.cpp
 * Copyright (C) 1999-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "tools.h"
#include "toolpalette_form.h"
#include "dbw3_form.h"
#include "language.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
/*-----------------------------------------------------------------*/
TToolpaletteForm *ToolpaletteForm;
bool toolpalette_loaded = false;
/*-----------------------------------------------------------------*/
__fastcall TToolpaletteForm::TToolpaletteForm(TComponent* Owner)
: TForm(Owner)
{
    toolpalette_loaded = true;
    ReloadLanguage();
    toolConstrained->Down = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolNormalClick(TObject *Sender)
{
    toolNormal->Down = true;
    DBWFRM->tool = TOOL_NORMAL;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolLineClick(TObject *Sender)
{
    toolLine->Down = true;
    DBWFRM->tool = TOOL_LINE;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolRectangleClick(TObject *Sender)
{
    toolRectangle->Down = true;
    DBWFRM->tool = TOOL_RECTANGLE;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolFilledRectangleClick(TObject *Sender)
{
    toolFilledRectangle->Down = true;
    DBWFRM->tool = TOOL_FILLEDRECTANGLE;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolEllipseClick(TObject *Sender)
{
    toolEllipse->Down = true;
    DBWFRM->tool = TOOL_ELLIPSE;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolFilledEllipseClick(TObject *Sender)
{
    toolFilledEllipse->Down = true;
    DBWFRM->tool = TOOL_FILLEDELLIPSE;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolLinesize1Click(TObject *Sender)
{
    toolLinesize1->Down = true;
    DBWFRM->linesize = LINE_SIZE1;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolLinesize2Click(TObject *Sender)
{
    toolLinesize2->Down = true;
    DBWFRM->linesize = LINE_SIZE2;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolLinesize3Click(TObject *Sender)
{
    toolLinesize3->Down = true;
    DBWFRM->linesize = LINE_SIZE3;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::toolLinesize4Click(TObject *Sender)
{
    toolLinesize4->Down = true;
    DBWFRM->linesize = LINE_SIZE4;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::FormHide(TObject *Sender)
{
    // Die verschiedenen Gewebeansichten einschalten
    DBWFRM->GewebeNone->Enabled = true;
    DBWFRM->GewebeFarbeffekt->Enabled = true;
    DBWFRM->GewebeSimulation->Enabled = true;
    DBWFRM->GewebeNonePopup->Enabled = true;
    DBWFRM->GewebeFarbeffektPopup->Enabled = true;
    DBWFRM->GewebeSimulationPopup->Enabled = true;
    // Tool auf Normal schalten
    toolNormal->Down = true;
    DBWFRM->tool = TOOL_NORMAL;
    // Menü ändern
    DBWFRM->ViewToolpalette->Checked = false;
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::FormKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)
{
    switch (Key) {
        case '1': DBWFRM->Range1Click(this); break;
        case '2': DBWFRM->Range2Click(this); break;
        case '3': DBWFRM->Range3Click(this); break;
        case '4': DBWFRM->Range4Click(this); break;
        case '5': DBWFRM->Range5Click(this); break;
        case '6': DBWFRM->Range6Click(this); break;
        case '7': DBWFRM->Range7Click(this); break;
        case '8': DBWFRM->Range8Click(this); break;
        case '9': DBWFRM->Range9Click(this); break;
        case VK_ESCAPE:
            Hide();
            break;
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            // Wahrscheinlich will Benutzer
            // Fokus auf Haupformular...
            DBWFRM->SetFocus();
            break;
    }
}
/*-----------------------------------------------------------------*/
void __fastcall TToolpaletteForm::ReloadLanguage()
{
    if (!toolpalette_loaded) return;
    LANG_C_H(this, EN, "Tools", "")
    LANG_C_H(this, GE, "Werkzeuge", "")
    LANG_C_H(toolNormal, EN, "", "Normal")
    LANG_C_H(toolNormal, GE, "", "Normal")
    LANG_C_H(toolLine, EN, "", "Normal")
    LANG_C_H(toolLine, GE, "", "Normal")
    LANG_C_H(toolRectangle, EN, "", "Rectangle")
    LANG_C_H(toolRectangle, GE, "", "Rechtecke")
    LANG_C_H(toolFilledRectangle, EN, "", "Filled Rectangle")
    LANG_C_H(toolFilledRectangle, GE, "", "Gefüllte Rechtecke")
    LANG_C_H(toolEllipse, EN, "", "Ellipse")
    LANG_C_H(toolEllipse, GE, "", "Ellipsen")
    LANG_C_H(toolFilledEllipse, EN, "", "Filled Ellipse")
    LANG_C_H(toolFilledEllipse, GE, "", "Gefüllte Ellipsen")
    LANG_C_H(toolConstrained, EN, "", "Squares and Circles")
    LANG_C_H(toolConstrained, GE, "", "Quadrate und Kreise")
    LANG_C_H(toolLinesize1, EN, "", "Very fine line")
    LANG_C_H(toolLinesize1, GE, "", "Sehr feine Linie")
    LANG_C_H(toolLinesize2, EN, "", "fine line")
    LANG_C_H(toolLinesize2, GE, "", "feine Linie")
    LANG_C_H(toolLinesize3, EN, "", "thick line")
    LANG_C_H(toolLinesize3, GE, "", "dicke Linie")
    LANG_C_H(toolLinesize4, EN, "", "Very thick line")
    LANG_C_H(toolLinesize4, GE, "", "Sehr dicke Linie")
}
/*-----------------------------------------------------------------*/


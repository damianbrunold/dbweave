/* DB-WEBEN, Textil CAD/CAM program for Windows 95/Windows NT 4
 * File pagesetup_form.cpp
 * Copyright (C) 1998-2000 Damian Brunold
 */
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#pragma hdrstop
/*-----------------------------------------------------------------*/
#include "pagesetup_form.h"
/*-----------------------------------------------------------------*/
#pragma package(smart_init)
#pragma resource "*.dfm"
TPageSetupForm *PageSetupForm;
/*-----------------------------------------------------------------*/
__fastcall TPageSetupForm::TPageSetupForm(TComponent* Owner)
	: TForm(Owner)
{
}
/*-----------------------------------------------------------------*/

/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "pagesetupdialog.h"
#include "mainwindow.h"
#include "datamodule.h"
#include "print.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QVBoxLayout>

/*-----------------------------------------------------------------*/
PageSetupDialog::PageSetupDialog (QWidget* _parent)
	: QDialog(_parent)
{
	setWindowTitle(QStringLiteral("Page setup"));
	setModal(true);

	auto makeMargin = [this]{
		auto* sb = new QDoubleSpinBox(this);
		sb->setRange(0.0, 10.0);
		sb->setSingleStep(0.1);
		sb->setDecimals(1);
		sb->setSuffix(QStringLiteral(" cm"));
		return sb;
	};
	left   = makeMargin();
	right  = makeMargin();
	top    = makeMargin();
	bottom = makeMargin();
	header = new QLineEdit(this);
	footer = new QLineEdit(this);

	auto* gb = new QGroupBox(QStringLiteral("Margins"), this);
	auto* gbl = new QFormLayout(gb);
	gbl->addRow(QStringLiteral("&Left:"),   left);
	gbl->addRow(QStringLiteral("&Right:"),  right);
	gbl->addRow(QStringLiteral("&Top:"),    top);
	gbl->addRow(QStringLiteral("&Bottom:"), bottom);

	auto* form = new QFormLayout();
	form->addRow(QStringLiteral("&Header:"), header);
	form->addRow(QStringLiteral("&Footer:"), footer);

	auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addWidget(gb);
	root->addLayout(form);
	root->addWidget(btns);

	resize(360, 260);
}

void PageSetupDialog::setMargins (int _l, int _r, int _t, int _b)
{
	left  ->setValue(_l / 10.0);
	right ->setValue(_r / 10.0);
	top   ->setValue(_t / 10.0);
	bottom->setValue(_b / 10.0);
}

int PageSetupDialog::leftTenths   () const { return int(left  ->value() * 10 + 0.5); }
int PageSetupDialog::rightTenths  () const { return int(right ->value() * 10 + 0.5); }
int PageSetupDialog::topTenths    () const { return int(top   ->value() * 10 + 0.5); }
int PageSetupDialog::bottomTenths () const { return int(bottom->value() * 10 + 0.5); }

void PageSetupDialog::setHeader (const QString& _s) { header->setText(_s); }
void PageSetupDialog::setFooter (const QString& _s) { footer->setText(_s); }
QString PageSetupDialog::headerText () const { return header->text(); }
QString PageSetupDialog::footerText () const { return footer->text(); }

/*-----------------------------------------------------------------*/
/*  TDBWFRM menu handlers for page setup / print / preview. */
void TDBWFRM::FileSetPageClick ()
{
	PageSetupDialog dlg(this);
	dlg.setMargins(borders.range.left,   borders.range.right,
	               borders.range.top,    borders.range.bottom);
	dlg.setHeader(header.text);
	dlg.setFooter(footer.text);
	if (dlg.exec() != QDialog::Accepted) return;
	borders.range.left   = dlg.leftTenths();
	borders.range.right  = dlg.rightTenths();
	borders.range.top    = dlg.topTenths();
	borders.range.bottom = dlg.bottomTenths();
	header.text = dlg.headerText();
	footer.text = dlg.footerText();
	SetModified();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::FilePrintClick ()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog pd(&printer, this);

	/*  Query page count for the FromPage/ToPage range by running a
	    dry layout pass on the live QPrinter. */
	{
		PrPrinterPrint probe(this, Data, &printer);
		const int pages = probe.GetPages();
		if (pages > 1) {
			pd.setOption(QAbstractPrintDialog::PrintPageRange, true);
			pd.setMinMax(1, pages);
		} else {
			pd.setOption(QAbstractPrintDialog::PrintPageRange, false);
		}
	}

	if (pd.exec() != QDialog::Accepted) return;

	PrPrinterPrint job(this, Data, &printer);
	if (pd.printRange() == QAbstractPrintDialog::PageRange)
		job.Print(pd.fromPage(), pd.toPage());
	else
		job.Print();
}

/*-----------------------------------------------------------------*/
void TDBWFRM::FilePrintpreviewClick ()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintPreviewDialog preview(&printer, this);
	connect(&preview, &QPrintPreviewDialog::paintRequested, this,
	        [this](QPrinter* _p){
		PrPrinterPreview job(this, Data, _p);
		job.Print();
	});
	preview.exec();
}

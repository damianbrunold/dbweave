/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy entwurfsinfo_form.cpp (TEntwurfsinfoForm).
    "Pattern information" — statistics over the current document:
    dimensions, colour distribution, heddle distribution, float
    distribution. Tree of four categories on the left, formatted
    report on the right; Print and Save (RTF/plaintext) buttons. */

#ifndef DBWEAVE_UI_ENTWURFSINFO_DIALOG_H
#define DBWEAVE_UI_ENTWURFSINFO_DIALOG_H

#include <QDialog>
#include <QStringList>

class QListWidget;
class QTextEdit;
class TDBWFRM;

class EntwurfsinfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EntwurfsinfoDialog(TDBWFRM* _frm, QWidget* _parent = nullptr);

    void initInfos();

private slots:
    void onCategoryChanged(int _row);
    void onPrint();
    void onExport();

private:
    TDBWFRM* frm = nullptr;

    QListWidget* categories = nullptr;
    QTextEdit* text = nullptr;

    QStringList ausmasse;
    QStringList farben;
    QStringList litzen;
    QStringList flottierungen;

    bool read_ausmasse = false;
    bool read_farben = false;
    bool read_litzen = false;
    bool read_flottierungen = false;

    void buildAusmasse();
    void buildFarben();
    void buildLitzen();
    void buildFlottierungen();

    void onAusmasse(bool _clear = true);
    void onFarben(bool _clear = true);
    void onLitzen(bool _clear = true);
    void onFlottierungen(bool _clear = true);

    void insertHeader();
    void insertHeading(const QString& _heading);
};

#endif

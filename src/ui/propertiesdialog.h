/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy properties_form.cpp (TFilePropertiesForm).
    Simple Author/Organization/Remarks metadata editor. */

#ifndef DBWEAVE_UI_PROPERTIES_DIALOG_H
#define DBWEAVE_UI_PROPERTIES_DIALOG_H

#include <QDialog>

class QLineEdit;
class QPlainTextEdit;

class PropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PropertiesDialog(QWidget* _parent = nullptr);

    QString author() const;
    QString organization() const;
    QString remarks() const;

    void setAuthor(const QString& _s);
    void setOrganization(const QString& _s);
    void setRemarks(const QString& _s);

private:
    QLineEdit* edAuthor = nullptr;
    QLineEdit* edOrganization = nullptr;
    QPlainTextEdit* edRemarks = nullptr;
};

#endif

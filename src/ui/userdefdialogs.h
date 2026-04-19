/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt ports of:
      - userdef_entername_form (TUserdefEnternameForm):
        UserdefEnterNameDialog + free function getUserdefName().
      - userdefselect_form (TUserdefSelectForm):
        UserdefSelectDialog — radio-button list of the 10 slots.    */

#ifndef DBWEAVE_UI_USERDEF_DIALOGS_H
#define DBWEAVE_UI_USERDEF_DIALOGS_H

#include <QDialog>
#include "userdef.h"

class QLineEdit;
class QRadioButton;

/*-----------------------------------------------------------------*/
class UserdefEnterNameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserdefEnterNameDialog(const QString& _default, QWidget* _parent = nullptr);
    QString name() const;

private:
    QLineEdit* edName = nullptr;
};

/*  Helper matching legacy free GetUserdefName() — returns the
    chosen name, or an empty string on Cancel. */
QString getUserdefName(QWidget* _parent, const QString& _default);

/*-----------------------------------------------------------------*/
class UserdefSelectDialog : public QDialog
{
    Q_OBJECT
public:
    UserdefSelectDialog(const UserdefPattern _slots[MAXUSERDEF], const QString& _title,
                        QWidget* _parent = nullptr);
    int selectedIndex() const;

private:
    QRadioButton* radios[MAXUSERDEF] = {};
};

#endif

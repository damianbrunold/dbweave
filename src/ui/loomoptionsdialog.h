/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  Qt port of legacy strgoptloom_form.cpp (TStrgOptLoomForm).
    Loom-type picker + serial-port picker. Built unconditionally
    so the UI offers the dropdown even when DBWEAVE_NO_LOOM is
    set; only the Dummy entry is live in that case and the other
    selections won't build a real controller.

    Where legacy offered a fixed COM1..COM8 list (its PORT enum),
    the port combo is now populated from QSerialPortInfo and is
    editable, so USB-serial adapters -- which routinely land on
    COM9+ on Windows and on /dev/ttyUSB* or /dev/cu.usbserial-* on
    Linux/macOS -- can actually be selected.                      */

#ifndef DBWEAVE_UI_LOOMOPTIONS_DIALOG_H
#define DBWEAVE_UI_LOOMOPTIONS_DIALOG_H

#include <QDialog>
#include <QString>

#include "loom.h" /* LOOMINTERFACE */

class QCheckBox;
class QComboBox;

class LoomOptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoomOptionsDialog(QWidget* _parent = nullptr);

    LOOMINTERFACE interf() const;

    /*  Platform-native device name, e.g. "COM3" or "/dev/ttyUSB0". */
    QString port() const;

    /*  Raise DTR/RTS after opening the port (legacy line state). */
    bool assertDtrRts() const;

    /*  Write a byte-level trace of the loom session. */
    bool trace() const;

    void setInterface(LOOMINTERFACE _i);
    void setPort(const QString& _p);
    void setAssertDtrRts(bool _on);
    void setTrace(bool _on);

private slots:
    void onInterfaceChanged(int _row);

private:
    QComboBox* cbInterface = nullptr;
    QComboBox* cbPort = nullptr;
    QCheckBox* chkAssertLines = nullptr;
    QCheckBox* chkTrace = nullptr;
    class QLabel* labTracePath = nullptr;
};

#endif

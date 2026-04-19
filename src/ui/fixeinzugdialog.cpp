/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "fixeinzugdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "draw_cell.h"
#include "cursor.h"
#include "einzug.h"

#include <QAction>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

#include <cstring>

/*-----------------------------------------------------------------*/
/*  Inner canvas widget: paints the threading grid and toggles a
    cell on click. Scroll offset is driven by an external QScrollBar
    because Data->MAXX1 can exceed the viewport width.              */
class FixeinzugCanvas : public QWidget
{
public:
    FixeinzugCanvas(FixeinzugDialog* _parent, TDBWFRM* _frm)
        : QWidget(_parent)
        , owner(_parent)
        , frm(_frm)
    {
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(192, 192, 192));
        setPalette(pal);
        setMinimumSize(400, 240);
        setFocusPolicy(Qt::StrongFocus);
        /*  Seed cursor direction from the main window's handler so
            keyboard stepping after Space matches the parent editor. */
        if (frm && frm->cursorhandler)
            cursordirection = frm->cursorhandler->GetCursorDirection();
    }

    int scrollX() const
    {
        return scrollx;
    }
    void setScrollX(int _s)
    {
        scrollx = _s;
        update();
    }

    int maxColsVisible() const
    {
        const int dx = cellSize();
        return dx > 0 ? width() / dx : 0;
    }

    int cellSize() const
    {
        return 16;
    }

    int maxShaftsShown() const
    {
        const int dy = cellSize();
        const int h = height();
        int m = dy > 0 ? h / dy : 0;
        if (m > Data->MAXY1)
            m = Data->MAXY1;
        return m;
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        const int dx = cellSize();
        const int dy = cellSize();
        const int maxx = maxColsVisible();
        const int maxy = maxShaftsShown();

        /*  Grid. */
        p.setPen(QColor(105, 105, 105));
        for (int i = 0; i <= maxx; i++)
            p.drawLine(i * dx, 0, i * dx, maxy * dy);
        for (int j = 0; j <= maxy; j++)
            p.drawLine(0, j * dy, maxx * dx, j * dy);

        /*  Cells. */
        for (int i = 0; i < maxx; i++) {
            const int src = scrollx + i;
            if (src >= Data->MAXX1)
                break;
            const short s = owner->scratch[src];
            if (s == 0)
                continue;
            if (s - 1 >= maxy)
                continue;
            const int x = frm->righttoleft ? (maxx - i - 1) * dx : i * dx;
            const int y = frm->toptobottom ? (s - 1) * dy : (maxy - s) * dy;
            PaintCell(p, frm->einzug.darstellung, x, y, x + dx, y + dy, QColor(Qt::black));
        }

        /*  Keyboard cursor: outlined rect at (cx, cy) with the same
            axis-flip rules the cells use. */
        if (hasFocus() && cx < maxx && cy < maxy) {
            const int x = frm->righttoleft ? (maxx - 1 - cx) * dx : cx * dx;
            const int y = frm->toptobottom ? cy * dy : (maxy - 1 - cy) * dy;
            p.setPen(QColor(Qt::white));
            p.setBrush(Qt::NoBrush);
            p.drawRect(x, y, dx, dy);
        }
    }

    void keyPressEvent(QKeyEvent* _e) override
    {
        const int maxx = maxColsVisible();
        const int maxy = maxShaftsShown();
        const bool ctrl = _e->modifiers().testFlag(Qt::ControlModifier);
        const int step = ctrl ? 4 : 1;

        auto moveLeft = [&] {
            cx -= step;
            if (cx < 0)
                cx = 0;
        };
        auto moveRight = [&] {
            cx += step;
            if (cx > maxx - 1)
                cx = maxx - 1;
        };
        auto moveUp = [&] {
            cy += step;
            if (cy > maxy - 1)
                cy = maxy - 1;
        };
        auto moveDown = [&] {
            cy -= step;
            if (cy < 0)
                cy = 0;
        };

        switch (_e->key()) {
        case Qt::Key_Left:
            if (frm->righttoleft)
                moveRight();
            else
                moveLeft();
            update();
            break;
        case Qt::Key_Right:
            if (frm->righttoleft)
                moveLeft();
            else
                moveRight();
            update();
            break;
        case Qt::Key_Up:
            if (frm->toptobottom)
                moveDown();
            else
                moveUp();
            update();
            break;
        case Qt::Key_Down:
            if (frm->toptobottom)
                moveUp();
            else
                moveDown();
            update();
            break;
        case Qt::Key_Space: {
            const int src = scrollx + cx;
            if (src >= Data->MAXX1)
                break;
            const short old = owner->scratch[src];
            owner->scratch[src] = (old == cy + 1) ? short(0) : short(cy + 1);
            owner->size = 0;
            for (int k = 0; k < Data->MAXX1; k++)
                if (owner->scratch[k] != 0)
                    owner->size = k;
            /*  Advance cursor in the configured direction. */
            if (frm->toptobottom) {
                if ((cursordirection & CD_DOWN) && cy > 0)
                    cy--;
                if ((cursordirection & CD_UP) && cy < maxy - 1)
                    cy++;
            } else {
                if ((cursordirection & CD_UP) && cy < maxy - 1)
                    cy++;
                if ((cursordirection & CD_DOWN) && cy > 0)
                    cy--;
            }
            if (frm->righttoleft) {
                if ((cursordirection & CD_RIGHT) && cx < maxx - 1)
                    cx++;
                if ((cursordirection & CD_LEFT) && cx > 0)
                    cx--;
            } else {
                if ((cursordirection & CD_LEFT) && cx > 0)
                    cx--;
                if ((cursordirection & CD_RIGHT) && cx < maxx - 1)
                    cx++;
            }
            update();
            break;
        }
        default:
            QWidget::keyPressEvent(_e);
            return;
        }
    }

    void mouseReleaseEvent(QMouseEvent* _e) override
    {
        if (_e->button() != Qt::LeftButton)
            return;
        const int dx = cellSize();
        const int dy = cellSize();
        const int maxx = maxColsVisible();
        const int maxy = maxShaftsShown();
        int i = _e->position().x() / dx;
        int j = maxy - 1 - (_e->position().y() / dy);
        if (i < 0 || j < 0)
            return;
        if (i >= maxx || j >= maxy)
            return;
        if (frm->righttoleft)
            i = maxx - 1 - i;
        if (frm->toptobottom)
            j = maxy - 1 - j;
        const int src = scrollx + i;
        if (src >= Data->MAXX1)
            return;
        const short old = owner->scratch[src];
        owner->scratch[src] = (old == j + 1) ? short(0) : short(j + 1);
        owner->size = 0;
        for (int k = 0; k < Data->MAXX1; k++)
            if (owner->scratch[k] != 0)
                owner->size = k;
        cx = i;
        cy = j;
        setFocus();
        update();
    }

private:
    FixeinzugDialog* owner;
    TDBWFRM* frm;
    int scrollx = 0;
    int cx = 0;
    int cy = 0;
    CURSORDIRECTION cursordirection = CD_RIGHT;
};

/*-----------------------------------------------------------------*/
FixeinzugDialog::FixeinzugDialog(TDBWFRM* _frm, QWidget* _parent)
    : QDialog(_parent)
    , frm(_frm)
{
    setWindowTitle(QStringLiteral("User defined threading"));
    setModal(true);
    resize(640, 300);

    scratch = new short[Data->MAXX1];
    std::memset(scratch, 0, sizeof(short) * Data->MAXX1);
    size = frm->fixsize;
    firstfree = frm->firstfree;
    for (int i = 0; i <= size && i < Data->MAXX1; i++)
        scratch[i] = frm->fixeinzug[i];
    calcRange();

    canvas = new FixeinzugCanvas(this, frm);
    canvas->setFocus();
    sbH = new QScrollBar(Qt::Horizontal, this);
    sbH->setRange(0, Data->MAXX1);
    sbH->setPageStep(32);
    connect(sbH, &QScrollBar::valueChanged, this, [this](int _v) { canvas->setScrollX(_v); });

    /*  Menu bar hosted on a QWidget parent (QDialog has no native
        menu-bar slot). */
    auto* menubar = new QMenuBar(this);
    QMenu* mEin = menubar->addMenu(QStringLiteral("&Threading"));
    QAction* aGrab = mEin->addAction(QStringLiteral("Take threading from &pattern"));
    QAction* aDel = mEin->addAction(QStringLiteral("&Delete"));
    mEin->addSeparator();
    QAction* aRevert = mEin->addAction(QStringLiteral("&Revert"));
    QAction* aClose = mEin->addAction(QStringLiteral("&Close"));
    connect(aGrab, &QAction::triggered, this, &FixeinzugDialog::onGrab);
    connect(aDel, &QAction::triggered, this, &FixeinzugDialog::onDelete);
    connect(aRevert, &QAction::triggered, this, &FixeinzugDialog::onRevert);
    connect(aClose, &QAction::triggered, this, &FixeinzugDialog::onClose);

    auto* root = new QVBoxLayout(this);
    root->setMenuBar(menubar);
    root->addWidget(canvas, 1);
    root->addWidget(sbH);
}

FixeinzugDialog::~FixeinzugDialog()
{
    delete[] scratch;
}

/*-----------------------------------------------------------------*/
void FixeinzugDialog::calcRange()
{
    size = 0;
    for (int i = 0; i < Data->MAXX1; i++)
        if (scratch[i] != 0)
            size = i;
}

short FixeinzugDialog::calcFirstFree() const
{
    short ff = 0;
    for (int i = 0; i < Data->MAXX1; i++)
        if (scratch[i] >= ff)
            ff = short(scratch[i]);
    return ff;
}

/*-----------------------------------------------------------------*/
void FixeinzugDialog::onGrab()
{
    /*  Reset scratch to the main window's current fixeinzug snapshot
        (which UpdateEinzugFixiert populated before the dialog opened
        or whenever the user invokes Grab again). */
    frm->UpdateEinzugFixiert();
    std::memset(scratch, 0, sizeof(short) * Data->MAXX1);
    size = frm->fixsize;
    firstfree = frm->firstfree;
    for (int i = 0; i <= size && i < Data->MAXX1; i++)
        scratch[i] = frm->fixeinzug[i];
    calcRange();
    canvas->update();
}

void FixeinzugDialog::onDelete()
{
    std::memset(scratch, 0, sizeof(short) * Data->MAXX1);
    size = 0;
    firstfree = 0;
    canvas->update();
}

void FixeinzugDialog::onRevert()
{
    reject();
}

void FixeinzugDialog::onClose()
{
    accept();
}

void FixeinzugDialog::accept()
{
    /*  Legacy rejects if any slot 0..size-1 is zero. */
    for (int i = 0; i < size; i++) {
        if (scratch[i] == 0) {
            QMessageBox::information(this, QStringLiteral("DB-WEAVE"),
                                     QStringLiteral("Threading must be continuous (no gaps)."));
            return;
        }
    }
    /*  Copy + rapport out to MAXX1-wide array. */
    std::memcpy(frm->fixeinzug, scratch, sizeof(short) * Data->MAXX1);
    frm->firstfree = calcFirstFree();
    frm->fixsize = size;
    int k = 0;
    for (int i = size + 1; i < Data->MAXX1; i++) {
        frm->fixeinzug[i] = frm->fixeinzug[k++];
        if (k > size)
            k = 0;
    }
    QDialog::accept();
}

/*-----------------------------------------------------------------*/
/*  TDBWFRM::UpdateEinzugFixiert -- verbatim port of legacy
    einzug.cpp:TDBWFRM::UpdateEinzugFixiert. Copies the current einzug
    into fixeinzug (skipping empty threads), rapports the pattern out
    to the end of MAXX1, and records the first free shaft index.   */
void TDBWFRM::UpdateEinzugFixiert()
{
    if (!fixeinzug)
        return;
    std::memset(fixeinzug, 0, Data->MAXX1 * sizeof(short));
    firstfree = 1;
    int ii = 0;
    for (int i = 0; i < Data->MAXX1; i++) {
        const short s = einzug.feld.Get(i);
        if (s != 0)
            fixeinzug[ii++] = s;
    }
    for (ii = Data->MAXX1 - 1; ii >= 0; ii--)
        if (fixeinzug[ii] != 0)
            break;
    int k = 0;
    fixsize = ii;
    for (int i = ii + 1; i < Data->MAXX1; i++) {
        fixeinzug[i] = fixeinzug[k++];
        if (k > ii)
            k = 0;
    }
    firstfree = 0;
    for (int i = Data->MAXY1 - 1; i >= 0; i--) {
        if (!freieschaefte[i]) {
            firstfree = short(i + 1);
            break;
        }
    }
}

/*-----------------------------------------------------------------*/
void TDBWFRM::EditFixeinzug()
{
    if (!fixeinzug)
        return;
    if (fixeinzug[0] == 0)
        UpdateEinzugFixiert();
    if (EzFixiert && !EzFixiert->isChecked())
        EzFixiert->setChecked(true);

    if (cursorhandler)
        cursorhandler->DisableCursor();
    FixeinzugDialog dlg(this, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (einzughandler)
            einzughandler->Fixiert();
        CalcRapport();
        UpdateStatusBar();
    }
    if (cursorhandler)
        cursorhandler->EnableCursor();
}

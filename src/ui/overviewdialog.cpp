/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

#include "overviewdialog.h"

#include "mainwindow.h"
#include "datamodule.h"
#include "palette.h"
#include "rangecolors.h"
#include "legacy_colors.h"
#include "colors_compat.h"
#include "language.h"

#include <QAction>
#include <QApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPageLayout>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

/*-----------------------------------------------------------------*/
/*  Inner canvas: paints the kette x schuesse rectangle of the
    document at the current zoom (1..10 px per cell). */
class OverviewCanvas : public QWidget
{
public:
    OverviewCanvas(OverviewDialog* _parent, TDBWFRM* _frm)
        : QWidget(_parent)
        , frm(_frm)
    {
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, legacyBtnFace());
        setPalette(pal);
        setMinimumSize(400, 300);
    }

    void setZoom(int _z)
    {
        zoom = _z;
        update();
    }
    int getZoom() const
    {
        return zoom;
    }

    void setGrid(bool _g)
    {
        grid = _g;
        update();
    }
    bool getGrid() const
    {
        return grid;
    }

    /*  Seed zoom from the current kette/schuesse extent so the entire
        pattern roughly fits into the canvas -- verbatim port of the
        legacy TOverviewForm ctor heuristic. Called from the first
        resizeEvent (after the WM has settled the maximized size). */
    void seedZoom()
    {
        const int kcount = frm->kette.count();
        const int scount = frm->schuesse.count();
        int ddx = kcount != 0 ? width() / kcount + 1 : 0;
        int ddy = scount != 0 ? height() / scount + 1 : 0;
        if (ddx <= 0)
            ddx = 1;
        if (ddy <= 0)
            ddy = 1;
        if (ddx > 10)
            ddx = 10;
        if (ddy > 10)
            ddy = 10;
        zoom = std::min(ddx, ddy);
        seeded = true;
    }

    bool isSeeded() const
    {
        return seeded;
    }

protected:
    void resizeEvent(QResizeEvent* _e) override
    {
        QWidget::resizeEvent(_e);
        /*  Lazy seed on first real resize -- under a tiling / maximising
            WM the constructor runs at the 800x600 fallback size, so we
            defer the heuristic until Qt gives us the final geometry. */
        if (!seeded) {
            seedZoom();
            update();
        }
    }

    void paintEvent(QPaintEvent*) override
    {
        if (frm->kette.a == -1 || frm->schuesse.a == -1)
            return;

        QPainter p(this);

        int gw = zoom;
        int gh = zoom;
        /*  Scale one axis up by the density factor ratio so the
            aspect matches the main editor's faktor_kette /
            faktor_schuss setting. */
        if (frm->faktor_kette > frm->faktor_schuss)
            gw = int(double(gw) * frm->faktor_kette / frm->faktor_schuss);
        else if (frm->faktor_schuss > frm->faktor_kette)
            gh = int(double(gh) * frm->faktor_schuss / frm->faktor_kette);
        /*  Don't exceed the main editor's own cell size -- overview
            is meant to be smaller, not larger. */
        if (frm->gewebe.gw > 0 && gw > frm->gewebe.gw)
            gw = frm->gewebe.gw;
        if (frm->gewebe.gh > 0 && gh > frm->gewebe.gh)
            gh = frm->gewebe.gh;

        const int W = width();
        const int H = height();
        const int mx = std::min(W / std::max(1, gw), frm->kette.count());
        const int my = std::min(H / std::max(1, gh), frm->schuesse.count());

        const QColor bkground = legacyBtnFace();
        p.fillRect(rect(), bkground);

        const QColor gridClr = legacyBtnShadow();

        const bool normalMode = frm->GewebeNormal && frm->GewebeNormal->isChecked();
        const bool farbeffektMode = frm->GewebeFarbeffekt && frm->GewebeFarbeffekt->isChecked();
        const bool simulationMode = frm->GewebeSimulation && frm->GewebeSimulation->isChecked();

        if (!normalMode && !farbeffektMode && !simulationMode)
            return;

        /*  Simulation rendering needs cells big enough to hold a
            warp strip + weft stubs; below that we fall back to the
            flat warp/weft Farbeffekt look.                        */
        const int dw = std::max(1, std::min(gw / 5, gw / 2 - 1));
        const int dh = std::max(1, std::min(gh / 5, gh / 2 - 1));
        const bool simDetailed = simulationMode && gw >= 5 && gh >= 5;

        for (int i = frm->kette.a; i < frm->kette.a + mx; i++) {
            for (int j = frm->schuesse.a; j < frm->schuesse.a + my; j++) {
                const char s = frm->gewebe.feld.Get(i, j);

                int x;
                if (frm->righttoleft)
                    x = W - (i - frm->kette.a + 1) * gw;
                else
                    x = (i - frm->kette.a) * gw;
                const int y = H - (j - frm->schuesse.a + 1) * gh;

                if (normalMode) {
                    /*  Patrone view: filled cells get their range
                        colour, empty cells just bkground. The grid is
                        controlled by the toolbar Raster toggle (legacy
                        behaviour).                                     */
                    if (s > 0) {
                        const QColor cell = qColorFromTColor(GetRangeColor(s));
                        if (gw > 1 || gh > 1) {
                            p.fillRect(x, y, gw, gh, cell);
                        } else {
                            p.setPen(cell);
                            p.drawPoint(x, y);
                            continue;
                        }
                    }
                    if (grid && gw > 2 && gh > 2) {
                        p.setPen(gridClr);
                        p.drawRect(x, y, gw, gh);
                    }
                    continue;
                }

                /*  Farbeffekt / Simulation share the warp-up vs
                    weft-up decision. */
                bool warpUp = (s > 0);
                if (frm->sinkingshed)
                    warpUp = !warpUp;
                const QColor kc = Data->palette ? qColorFromTColor(Data->palette->GetColor(
                                                      int(frm->kettfarben.feld.Get(i))))
                                                : QColor(Qt::white);
                const QColor sc = Data->palette ? qColorFromTColor(Data->palette->GetColor(
                                                      int(frm->schussfarben.feld.Get(j))))
                                                : QColor(Qt::white);

                if (gw <= 1 && gh <= 1) {
                    p.setPen(warpUp ? kc : sc);
                    p.drawPoint(x, y);
                    continue;
                }

                if (simDetailed) {
                    /*  Mirrors TDBWFRM::DrawGewebeSimulation: paint a
                        thread strip in the dominant fibre's colour and
                        leave small "stubs" of the cross fibre showing
                        at the cell edges.                             */
                    p.fillRect(x, y, gw, gh, legacyBtnFace());
                    if (warpUp) {
                        p.fillRect(x + dw, y, gw - 2 * dw, gh, kc);
                        p.fillRect(x, y + dh, dw, gh - 2 * dh, sc);
                        p.fillRect(x + gw - dw, y + dh, dw, gh - 2 * dh, sc);
                        p.setPen(QColor(Qt::black));
                        p.drawLine(x + gw - dw, y + dh, x + gw - dw, y + gh - dh);
                    } else {
                        p.fillRect(x, y + dh, gw, gh - 2 * dh, sc);
                        p.fillRect(x + dw, y, gw - 2 * dw, dh, kc);
                        p.fillRect(x + dw, y + gh - dh, gw - 2 * dw, dh, kc);
                        p.setPen(QColor(Qt::black));
                        p.drawLine(x + dw, y + gh - dh, x + gw - dw, y + gh - dh);
                    }
                } else {
                    p.fillRect(x, y, gw, gh, warpUp ? kc : sc);
                }
                /*  Simulation cells never carry a grid (matches main
                    canvas); Farbeffekt cells only when the Raster
                    toggle (seeded from fewithraster) is on.         */
                if (farbeffektMode && grid && gw > 2 && gh > 2) {
                    p.setPen(gridClr);
                    p.drawRect(x, y, gw, gh);
                }
            }
        }
    }

private:
    TDBWFRM* frm;
    int zoom = 4;
    bool grid = true;
    bool seeded = false;
};

/*-----------------------------------------------------------------*/
OverviewDialog::OverviewDialog(TDBWFRM* _frm, QWidget* _parent)
    : QDialog(_parent)
    , frm(_frm)
{
    setWindowTitle(LANG_STR("Pattern overview - DB-WEAVE", "Bindungsübersicht - DB-WEAVE"));
    /*  Legacy opened this form maximized (dfm: WindowState=wsMaximized).
        QDialog by default hides the minimize/maximize buttons, and many
        window managers then refuse setWindowState(WindowMaximized) on a
        dialog; add the max/min hints explicitly, then call
        showMaximized() in showEvent so the WM has something to honour.
        The 800x600 resize is the unmaximized fallback geometry.     */
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    resize(800, 600);

    auto* toolbar = new QToolBar(this);
    actZoomOut = toolbar->addAction(LANG_STR("Zoom &out", "Zoom ver&kleinern"), this, [this] {
        if (canvas->getZoom() > 1) {
            canvas->setZoom(canvas->getZoom() - 1);
            updateZoomActions();
        }
    });
    actZoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
    actZoomIn = toolbar->addAction(LANG_STR("Zoom &in", "Zoom ver&grössern"), this, [this] {
        if (canvas->getZoom() < 10) {
            canvas->setZoom(canvas->getZoom() + 1);
            updateZoomActions();
        }
    });
    actZoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    actGrid = toolbar->addAction(LANG_STR("&Grid", "&Raster"), this, [this] {
        canvas->setGrid(!canvas->getGrid());
        actGrid->setChecked(canvas->getGrid());
    });
    actGrid->setCheckable(true);
    actGrid->setChecked(true);
    actGrid->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    actPrint
        = toolbar->addAction(LANG_STR("&Print...", "&Drucken..."), this, [this] { doPrint(); });
    actPrint->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    toolbar->addSeparator();
    QAction* actClose
        = toolbar->addAction(LANG_STR("&Close", "&Schliessen"), this, &QDialog::accept);
    Q_UNUSED(actClose);

    canvas = new OverviewCanvas(this, frm);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(toolbar);
    root->addWidget(canvas, 1);

    /*  Zoom is seeded lazily from OverviewCanvas::resizeEvent once
        the final (maximised) geometry has settled; updateZoomActions
        runs there too via the zoom getters each time the user clicks
        a zoom button.                                              */
    updateZoomActions();
}

void OverviewDialog::keyPressEvent(QKeyEvent* _e)
{
    if (_e->key() == Qt::Key_Escape || _e->key() == Qt::Key_F4) {
        accept();
        return;
    }
    /*  Mirror the main editor's Ctrl+1/2/3 view-mode shortcuts so the
        user can switch between Patrone / Farbeffekt / Simulation
        without leaving the overview. Triggering the QAction also
        updates the main canvas via the existing connection.          */
    if (_e->modifiers() == Qt::ControlModifier) {
        QAction* target = nullptr;
        switch (_e->key()) {
        case Qt::Key_1:
            target = frm->GewebeNormal;
            break;
        case Qt::Key_2:
            target = frm->GewebeFarbeffekt;
            break;
        case Qt::Key_3:
            target = frm->GewebeSimulation;
            break;
        default:
            break;
        }
        if (target) {
            target->trigger();
            canvas->update();
            return;
        }
    }
    QDialog::keyPressEvent(_e);
}

void OverviewDialog::showEvent(QShowEvent* _e)
{
    QDialog::showEvent(_e);
    /*  Ask the WM to maximise now that the dialog is actually shown.
        Setting the state in the ctor is unreliable for QDialog on
        many linux WMs; showMaximized() here works because the window
        is already mapped when we call it. The canvas seeds its zoom
        lazily from its first resizeEvent, so the heuristic sees the
        final maximised geometry rather than the 800x600 fallback. */
    if (!(windowState() & Qt::WindowMaximized))
        showMaximized();
}

/*  Port of legacy TOverviewForm::SBPrintClick. Tiles the
    kette × schuesse gewebe onto the printer in the same colour mode
    the editor is currently showing (GewebeNormal = range colours,
    Farbeffekt / Simulation = warp/weft palette colour). The cell
    size mirrors legacy CalcPrintDimensions: 2/10 mm per zoom unit,
    so zoom 5 produces 1 mm cells.                                  */
void OverviewDialog::doPrint()
{
    if (frm->kette.a < 0 || frm->schuesse.a < 0)
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setDocName(QStringLiteral("DB-WEAVE - ") + QFileInfo(frm->filename).fileName());
    QPrintDialog pd(&printer, this);
    if (pd.exec() != QDialog::Accepted)
        return;

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QPainter p(&printer);

    const int dpix = int(printer.logicalDpiX());
    const int dpiy = int(printer.logicalDpiY());

    /*  mm → device pixels. Legacy formula: mm * 10 * dpi / 254.    */
    auto mmToPxX = [dpix](int _mm) { return _mm * 10 * dpix / 254; };
    auto mmToPxY = [dpiy](int _mm) { return _mm * 10 * dpiy / 254; };

    const int mleft = mmToPxX(frm->borders.range.left);
    const int mright = mmToPxX(frm->borders.range.right);
    const int mtop = mmToPxY(frm->borders.range.top + frm->header.height);
    const int mbottom = mmToPxY(frm->borders.range.bottom + frm->footer.height);

    const int pageW = printer.pageLayout().paintRectPixels(printer.resolution()).width();
    const int pageH = printer.pageLayout().paintRectPixels(printer.resolution()).height();
    const int pwidth = pageW - mleft - mright;
    const int pheight = pageH - mtop - mbottom;
    if (pwidth < 1 || pheight < 1) {
        p.end();
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    /*  Cell size in printer pixels: legacy `2*zoom*dpi/254` gives
        zoom/5 mm per cell, then the weft/warp ratio stretches the
        larger-factor axis.                                         */
    const int z = canvas->getZoom();
    int gw = 2 * z * dpix / 254;
    int gh = 2 * z * dpiy / 254;
    if (gw < 1)
        gw = 1;
    if (gh < 1)
        gh = 1;
    const float fk = frm->faktor_kette;
    const float fs = frm->faktor_schuss;
    if (fk > 0.0f && fs > 0.0f) {
        if (fk > fs)
            gw = int(double(gw) * fk / fs);
        else if (fs > fk)
            gh = int(double(gh) * fs / fk);
    }

    const int mx = std::min(pwidth / gw, frm->kette.count());
    const int my = std::min(pheight / gh, frm->schuesse.count());

    const bool normalMode = frm->GewebeNormal && frm->GewebeNormal->isChecked();
    const bool coloredMode = (frm->GewebeFarbeffekt && frm->GewebeFarbeffekt->isChecked())
                             || (frm->GewebeSimulation && frm->GewebeSimulation->isChecked());
    const bool grid = canvas->getGrid();

    for (int i = frm->kette.a; i < frm->kette.a + mx; i++) {
        for (int j = frm->schuesse.a; j < frm->schuesse.a + my; j++) {
            const char s = frm->gewebe.feld.Get(i, j);
            QColor cell;
            bool draw = true;
            if (normalMode) {
                if (s > 0)
                    cell = qColorFromTColor(GetRangeColor(s));
                else
                    draw = false;
            } else if (coloredMode) {
                bool warpUp = (s > 0);
                if (frm->sinkingshed)
                    warpUp = !warpUp;
                const int cIdx = warpUp ? int(frm->kettfarben.feld.Get(i))
                                        : int(frm->schussfarben.feld.Get(j));
                cell = Data->palette ? qColorFromTColor(Data->palette->GetColor(cIdx))
                                     : QColor(Qt::white);
            } else {
                draw = false;
            }

            int x;
            if (frm->righttoleft)
                x = pwidth - (i - frm->kette.a + 1) * gw;
            else
                x = (i - frm->kette.a) * gw;
            const int y = (my + 1) * gh - (j - frm->schuesse.a + 1) * gh;

            if (draw)
                p.fillRect(mleft + x, mtop + y, gw, gh, cell);
            if (grid && gw > 2 && gh > 2) {
                p.setPen(Qt::black);
                p.drawRect(mleft + x, mtop + y, gw, gh);
            }
        }
    }

    p.end();
    QGuiApplication::restoreOverrideCursor();
    QApplication::beep();
}

void OverviewDialog::updateZoomActions()
{
    const int z = canvas->getZoom();
    actZoomOut->setEnabled(z > 1);
    actZoomIn->setEnabled(z < 10);
}

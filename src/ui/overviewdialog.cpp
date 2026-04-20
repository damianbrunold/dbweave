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
#include <QKeyEvent>
#include <QPainter>
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
        const bool coloredMode = (frm->GewebeFarbeffekt && frm->GewebeFarbeffekt->isChecked())
                                 || (frm->GewebeSimulation && frm->GewebeSimulation->isChecked());

        if (!normalMode && !coloredMode)
            return;

        for (int i = frm->kette.a; i < frm->kette.a + mx; i++) {
            for (int j = frm->schuesse.a; j < frm->schuesse.a + my; j++) {
                const char s = frm->gewebe.feld.Get(i, j);
                QColor cell;
                bool draw = true;
                if (normalMode) {
                    if (s > 0) {
                        cell = qColorFromTColor(GetRangeColor(s));
                    } else {
                        draw = false;
                    }
                } else {
                    /*  Farbeffekt / Simulation: warp colour when the
                        warp is "up" (gewebe > 0, xor sinkingshed),
                        otherwise the weft colour.                 */
                    bool warpUp = (s > 0);
                    if (frm->sinkingshed)
                        warpUp = !warpUp;
                    const int cIdx = warpUp ? int(frm->kettfarben.feld.Get(i))
                                            : int(frm->schussfarben.feld.Get(j));
                    cell = Data->palette ? qColorFromTColor(Data->palette->GetColor(cIdx))
                                         : QColor(Qt::white);
                }

                int x;
                if (frm->righttoleft)
                    x = W - (i - frm->kette.a + 1) * gw;
                else
                    x = (i - frm->kette.a) * gw;
                const int y = H - (j - frm->schuesse.a + 1) * gh;

                if (gw > 1 || gh > 1) {
                    if (draw) {
                        p.fillRect(x, y, gw, gh, cell);
                    }
                    /*  Thin shadow grid when the cells are large
                        enough to hold visible borders.            */
                    if (grid && gw > 2 && gh > 2) {
                        p.setPen(gridClr);
                        p.drawRect(x, y, gw, gh);
                    }
                } else if (draw) {
                    p.setPen(cell);
                    p.drawPoint(x, y);
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

void OverviewDialog::updateZoomActions()
{
    const int z = canvas->getZoom();
    actZoomOut->setEnabled(z > 1);
    actZoomIn->setEnabled(z < 10);
}

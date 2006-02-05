/* This file is part of the KDE libraries
   Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTimer>

#include <kconfig.h>
#include <kglobal.h>

#include "kscrollview.h"

struct KScrollView::KScrollViewPrivate {
    KScrollViewPrivate() : dx(0), dy(0), ddx(0), ddy(0), rdx(0), rdy(0), scrolling(false) {}
    QTimer timer;
    int dx;
    int dy;
    // Step size * 16 and residual to avoid huge difference between 1px/step and 2px/step
    int ddx;
    int ddy;
    int rdx;
    int rdy;
    bool scrolling;
};

KScrollView::KScrollView( QWidget *parent )
    : QScrollArea( parent ), d(new KScrollViewPrivate)
{
    connect(&d->timer, SIGNAL(timeout()), this, SLOT(scrollTick()));
}

KScrollView::~KScrollView()
{
    delete d;
}

void KScrollView::scrollContentsBy(int dx, int dy)
{
    KConfigGroup cfg( KGlobal::config(), "KDE" );
    if( !cfg.readEntry( "SmoothScrolling", true ) ) {
        QScrollArea::scrollContentsBy( dx, dy );
        return;
    }
    // scrolling destination
    int full_dx = d->dx + dx;
    int full_dy = d->dy + dy;

    // scrolling speed
    int ddx = 0;
    int ddy = 0;

    int steps = SCROLL_TIME/SCROLL_TICK;

    ddx = (full_dx*16)/steps;
    ddy = (full_dy*16)/steps;

    // don't go under 1px/step
    if (ddx > 0 && ddx < 16) ddx = 16;
    if (ddy > 0 && ddy < 16) ddy = 16;
    if (ddx < 0 && ddx > -16) ddx = -16;
    if (ddy < 0 && ddy > -16) ddy = -16;

    d->dx = full_dx;
    d->dy = full_dy;
    d->ddx = ddx;
    d->ddy = ddy;

    if (!d->scrolling) {
        scrollTick();
        startScrolling();
    }
}

void KScrollView::scrollTick() {
    if (d->dx == 0 && d->dy == 0) {
        stopScrolling();
        return;
    }

    int tddx = d->ddx + d->rdx;
    int tddy = d->ddy + d->rdy;

    int ddx = tddx / 16;
    int ddy = tddy / 16;
    d->rdx = tddx % 16;
    d->rdy = tddy % 16;

    if (d->dx > 0 && ddx > d->dx) ddx = d->dx;
    else
    if (d->dx < 0 && ddx < d->dx) ddx = d->dx;

    if (d->dy > 0 && ddy > d->dy) ddy = d->dy;
    else
    if (d->dy < 0 && ddy < d->dy) ddy = d->dy;

    d->dx -= ddx;
    d->dy -= ddy;

    QScrollArea::scrollContentsBy(ddx, ddy);
}

void KScrollView::startScrolling()
{
    d->scrolling = true;
    d->timer.start(SCROLL_TICK);
}

void KScrollView::stopScrolling()
{
    d->timer.stop();
    d->dx = d->dy = 0;
    d->scrolling = false;
}

// Overloaded from QScrollArea and QScrollBar
void KScrollView::wheelEvent( QWheelEvent *e )
{
    int pageStep = verticalScrollBar()->pageStep();
    int lineStep = verticalScrollBar()->singleStep();
    int step = qMin( QApplication::wheelScrollLines()*lineStep, pageStep );
    if ( ( e->modifiers() & Qt::ControlModifier ) || ( e->modifiers() & Qt::ShiftModifier ) )
        step = pageStep;

    int dy = (e->delta()*step)/120;
    scrollContentsBy(0,-dy);
    e->accept();
}

#include "kscrollview.moc"

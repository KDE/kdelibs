/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kfadewidgeteffect.h"
#include "kfadewidgeteffect_p.h"

#include <config.h> // for HAVE_XRENDER

#include <QtCore/QEvent>
#include <QtGui/QPaintEngine>
#include <QtGui/QPainter>

#include <kglobalsettings.h>

#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
#  include <X11/Xlib.h>
#  include <X11/extensions/Xrender.h>
#  include <QX11Info>
#  undef KeyPress
#  undef FocusOut
#endif

KFadeWidgetEffectPrivate::KFadeWidgetEffectPrivate(QWidget *_destWidget)
    : destWidget(_destWidget), disabled(false)
{
}

// Code from KFileItemDelegate (Author: Frederik Höglund)
// Fast transitions. Read:
// http://techbase.kde.org/Development/Tutorials/Graphics/Performance
// for further information on why not use setOpacity.
QPixmap KFadeWidgetEffectPrivate::transition(const QPixmap &from, const QPixmap &to, qreal amount) const
{
    const int value = int(0xff * amount);

    if (value == 0)
        return from;

    if (value == 1)
        return to;

    QColor color;
    color.setAlphaF(amount);

    // If the native paint engine supports Porter/Duff compositing and CompositionMode_Plus
    if (from.paintEngine()->hasFeature(QPaintEngine::PorterDuff) &&
        from.paintEngine()->hasFeature(QPaintEngine::BlendModes))
    {
        QPixmap under = from;
        QPixmap over  = to;

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawPixmap(0, 0, over);
        p.end();

        return under;
    }
#if defined(Q_WS_X11) && defined(HAVE_XRENDER)
    else if (from.paintEngine()->hasFeature(QPaintEngine::PorterDuff)) // We have Xrender support
    {
        // QX11PaintEngine doesn't implement CompositionMode_Plus in Qt 4.3,
        // which we need to be able to do a transition from one pixmap to
        // another.
        //
        // In order to avoid the overhead of converting the pixmaps to images
        // and doing the operation entirely in software, this function has a
        // specialized path for X11 that uses Xrender directly to do the
        // transition. This operation can be fully accelerated in HW.
        //
        // This specialization can be removed when QX11PaintEngine supports
        // CompositionMode_Plus.
        QPixmap source(to), destination(from);

        source.detach();
        destination.detach();

        Display *dpy = QX11Info::display();

        XRenderPictFormat *format = XRenderFindStandardFormat(dpy, PictStandardA8);
        XRenderPictureAttributes pa;
        pa.repeat = 1; // RepeatNormal

        // Create a 1x1 8 bit repeating alpha picture
        Pixmap pixmap = XCreatePixmap(dpy, destination.handle(), 1, 1, 8);
        Picture alpha = XRenderCreatePicture(dpy, pixmap, format, CPRepeat, &pa);
        XFreePixmap(dpy, pixmap);

        // Fill the alpha picture with the opacity value
        XRenderColor xcolor;
        xcolor.alpha = quint16(0xffff * amount);
        XRenderFillRectangle(dpy, PictOpSrc, alpha, &xcolor, 0, 0, 1, 1);

        // Reduce the alpha of the destination with 1 - opacity
        XRenderComposite(dpy, PictOpOutReverse, alpha, None, destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        // Add source * opacity to the destination
        XRenderComposite(dpy, PictOpAdd, source.x11PictureHandle(), alpha,
                         destination.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, destination.width(), destination.height());

        XRenderFreePicture(dpy, alpha);
        return destination;
    }
#endif
    else
    {
        // Fall back to using QRasterPaintEngine to do the transition.
        QImage under = from.toImage();
        QImage over  = to.toImage();

        QPainter p;
        p.begin(&over);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(over.rect(), color);
        p.end();

        p.begin(&under);
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        p.fillRect(under.rect(), color);
        p.setCompositionMode(QPainter::CompositionMode_Plus);
        p.drawImage(0, 0, over);
        p.end();

        return QPixmap::fromImage(under);
    }
}

KFadeWidgetEffect::KFadeWidgetEffect(QWidget *destWidget)
    : QWidget(destWidget ? destWidget->parentWidget() : 0),
    d_ptr(new KFadeWidgetEffectPrivate(destWidget))
{
    Q_D(KFadeWidgetEffect);
    d->q_ptr = this;
    Q_ASSERT(destWidget && destWidget->parentWidget());
    if (!destWidget || !destWidget->parentWidget() || !destWidget->isVisible() ||
        !(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        d->disabled = true;
        hide();
        return;
    }
    setGeometry(QRect(destWidget->mapTo(parentWidget(), QPoint(0, 0)), destWidget->size()));
    d->oldPixmap = QPixmap::grabWidget(destWidget);
    d->timeLine.setFrameRange(0, 255);
    d->timeLine.setCurveShape(QTimeLine::EaseOutCurve);
    connect(&d->timeLine, SIGNAL(finished()), SLOT(finished()));
    connect(&d->timeLine, SIGNAL(frameChanged(int)), SLOT(repaint()));
    show();
}

KFadeWidgetEffect::~KFadeWidgetEffect()
{
    delete d_ptr;
}

void KFadeWidgetEffectPrivate::finished()
{
    Q_Q(KFadeWidgetEffect);
    destWidget->setUpdatesEnabled(false);
    q->hide();
    q->deleteLater();
    destWidget->setUpdatesEnabled(true);
}

void KFadeWidgetEffect::start(int duration)
{
    Q_D(KFadeWidgetEffect);
    if (d->disabled) {
        deleteLater();
        return;
    }
    d->newPixmap = QPixmap::grabWidget(d->destWidget);
    d->timeLine.setDuration(duration);
    d->timeLine.start();
}

void KFadeWidgetEffect::paintEvent(QPaintEvent *)
{
    Q_D(KFadeWidgetEffect);
    QPainter p(this);
    p.drawPixmap(rect(), d->transition(d->oldPixmap, d->newPixmap, d->timeLine.currentValue()));
    p.end();
}

#include "moc_kfadewidgeteffect.cpp"

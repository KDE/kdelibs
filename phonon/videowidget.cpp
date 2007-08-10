/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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

#include "videowidget.h"
#include "videowidget_p.h"
#include "videowidgetinterface.h"
#include "factory.h"
#include "phonondefs_p.h"
#include "phononnamespace_p.h"

#include <QtGui/QAction>

#define PHONON_INTERFACENAME VideoWidgetInterface

namespace Phonon
{

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , Phonon::AbstractVideoOutput(*new VideoWidgetPrivate(this))
{
    K_D(VideoWidget);
    d->init();
    d->createBackendObject();
}

VideoWidget::VideoWidget(VideoWidgetPrivate &dd, QWidget *parent)
    : QWidget(parent),
    Phonon::AbstractVideoOutput(dd)
{
    K_D(VideoWidget);
    d->init();
}

void VideoWidgetPrivate::init()
{
    Q_Q(VideoWidget);
    QObject::connect(&cursorTimer, SIGNAL(timeout()), q, SLOT(_k_cursorTimeout()));
    cursorTimer.start();
    q->setMouseTracking(true);
}

void VideoWidget::mouseMoveEvent(QMouseEvent *)
{
    K_D(VideoWidget);
    if (k_ptr->backendObject()) {
        QWidget *w = INTERFACE_CALL(widget());
        if (w && Qt::BlankCursor == w->cursor().shape()) {
            w->unsetCursor();
            d->cursorTimer.start();
        }
    }
}

void VideoWidgetPrivate::_k_cursorTimeout()
{
    if (m_backendObject) {
        QWidget *w = pINTERFACE_CALL(widget());
        if (w && Qt::ArrowCursor == w->cursor().shape()) {
            w->setCursor(Qt::BlankCursor);
        }
    }
}
void VideoWidgetPrivate::createBackendObject()
{
    if (m_backendObject)
        return;
    Q_Q(VideoWidget);
    m_backendObject = Factory::createVideoWidget(q);
    if (m_backendObject) {
        setupBackendObject();
    }
}

#define PHONON_CLASSNAME VideoWidget

PHONON_INTERFACE_GETTER(Phonon::VideoWidget::AspectRatio, aspectRatio, d->aspectRatio)
PHONON_INTERFACE_SETTER(setAspectRatio, aspectRatio, Phonon::VideoWidget::AspectRatio)

PHONON_INTERFACE_GETTER(Phonon::VideoWidget::ScaleMode, scaleMode, d->scaleMode)
PHONON_INTERFACE_SETTER(setScaleMode, scaleMode, Phonon::VideoWidget::ScaleMode)

PHONON_INTERFACE_GETTER(qreal, brightness, d->brightness)
PHONON_INTERFACE_SETTER(setBrightness, brightness, qreal)

PHONON_INTERFACE_GETTER(qreal, contrast, d->contrast)
PHONON_INTERFACE_SETTER(setContrast, contrast, qreal)

PHONON_INTERFACE_GETTER(qreal, hue, d->hue)
PHONON_INTERFACE_SETTER(setHue, hue, qreal)

PHONON_INTERFACE_GETTER(qreal, saturation, d->saturation)
PHONON_INTERFACE_SETTER(setSaturation, saturation, qreal)

void VideoWidget::setFullScreen(bool newFullScreen)
{
    pDebug() << Q_FUNC_INFO << newFullScreen;
    K_D(VideoWidget);
    // TODO: disable screensaver? or should we leave that responsibility to the
    // application?
    Qt::WindowFlags flags = windowFlags();
    if (newFullScreen) {
        d->changeFlags = !(flags & Qt::Window);
        if (d->changeFlags) {
            flags &= ~Qt::SubWindow;
            flags |= Qt::Window;
        } // else it's a toplevel window already
    } else if (d->changeFlags && parent()) {
        flags &= ~Qt::Window;
        flags |= Qt::SubWindow;
    }
    setWindowFlags(flags);
    if (newFullScreen) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void VideoWidget::exitFullScreen()
{
    setFullScreen(false);
}

void VideoWidget::enterFullScreen()
{
    setFullScreen(true);
}

bool VideoWidgetPrivate::aboutToDeleteBackendObject()
{
    aspectRatio = pINTERFACE_CALL(aspectRatio());
    scaleMode = pINTERFACE_CALL(scaleMode());
    return AbstractVideoOutputPrivate::aboutToDeleteBackendObject();
}

void VideoWidgetPrivate::setupBackendObject()
{
    Q_Q(VideoWidget);
    Q_ASSERT(m_backendObject);
    //AbstractVideoOutputPrivate::setupBackendObject();
    pDebug() << "calling setAspectRatio on the backend " << aspectRatio;
    pINTERFACE_CALL(setAspectRatio(aspectRatio));
    pINTERFACE_CALL(setScaleMode(scaleMode));

    QWidget *w = pINTERFACE_CALL(widget());
    if (w) {
        layout.addWidget(w);
        q->setSizePolicy(w->sizePolicy());
    }
}

/*
QSize VideoWidget::sizeHint()
{
    if (k_ptr->backendObject())
    {
        QWidget *w = pINTERFACE_CALL(widget());
        if (w)
            return w->sizeHint();
    }
    return QSize(0, 0);
}

QSize VideoWidget::minimumSizeHint()
{
    if (k_ptr->backendObject())
    {
        QWidget *w = pINTERFACE_CALL(widget());
        if (w)
            return w->minimumSizeHint();
    }
    return QSize(0, 0);
} */

} //namespace Phonon

#include "moc_videowidget.cpp"

#undef PHONON_CLASSNAME
// vim: sw=4 ts=4 tw=80

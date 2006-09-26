/* This file is part of the KDE libraries
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "kiconengine.h"

#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QApplication>

#include <kiconloader.h>

class KIconEnginePrivate
{
public:
    QString iconName;
    int overlays;
    KIconLoader* iconLoader;
};

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader, int overlays)
    : d(new KIconEnginePrivate)
{
    d->iconName = iconName;
    d->iconLoader = iconLoader;
    d->overlays = overlays;
}

KIconEngine::~KIconEngine()
{
    delete d;
}

static int qIconModeToKIconState( QIcon::Mode mode )
{
    int kstate;
    switch (mode) {
    default:
    case QIcon::Normal:
        kstate = K3Icon::DefaultState;
        break;
    case QIcon::Active:
        kstate = K3Icon::ActiveState;
        break;
    case QIcon::Disabled:
        kstate = K3Icon::DisabledState;
        break;
    }
    return kstate;
}

QSize KIconEngine::actualSize( const QSize & size, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)

    const int kstate = qIconModeToKIconState(mode);
    // We ignore overlays here

    QPixmap pix = iconLoader()->loadIcon(d->iconName, K3Icon::Desktop, qMin(size.width(), size.height()), kstate);
    return pix.size();
}

void KIconEngine::paint( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)

    const int kstate = qIconModeToKIconState(mode) | d->overlays;
    K3Icon::Group group = K3Icon::Desktop;

    if (QWidget* targetWidget = dynamic_cast<QWidget*>(painter->device())) {
        if (qobject_cast<QMenu*>(targetWidget))
            group = K3Icon::Small;
        else if (qobject_cast<QToolBar*>(targetWidget->parent()))
            group = K3Icon::Toolbar;
    }

    QPixmap pix = iconLoader()->loadIcon(d->iconName, group, qMin(rect.width(), rect.height()), kstate);

    painter->drawPixmap(rect, pix);
}

const QString & KIconEngine::iconName( ) const
{
    return d->iconName;
}

KIconLoader * KIconEngine::iconLoader( ) const
{
    return d->iconLoader;
}

QPixmap KIconEngine::pixmap( const QSize & size, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)

    QPixmap pix(size);
    pix.fill(QColor(0,0,0,0));

    QPainter painter(&pix);

    const int kstate = qIconModeToKIconState(mode) | d->overlays;

    painter.drawPixmap(QPoint(), iconLoader()->loadIcon(d->iconName, K3Icon::Desktop,
                                                        qMin(size.width(), size.height()), kstate));

    return pix;
}

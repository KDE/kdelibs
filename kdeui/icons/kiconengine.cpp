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

#include "kiconengine_p.h"

#include <QPainter>
#include <QMenu>
#include <QToolBar>
#include <QApplication>

#include <kiconloader.h>

class KIconEngine::Private
{
public:
    QString iconName;
    QStringList overlays;
    KIconLoader* iconLoader;
};

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader, const QStringList &overlays)
    : d(new Private)
{
    d->iconName = iconName;
    d->iconLoader = iconLoader;
    d->overlays = overlays;
}

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader)
    : d(new Private)
{
    d->iconName = iconName;
    d->iconLoader = iconLoader;
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
        kstate = KIconLoader::DefaultState;
        break;
    case QIcon::Active:
        kstate = KIconLoader::ActiveState;
        break;
    case QIcon::Disabled:
        kstate = KIconLoader::DisabledState;
        break;
    }
    return kstate;
}

QSize KIconEngine::actualSize( const QSize & size, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)
    const int kstate = qIconModeToKIconState(mode);
    const QPixmap pix = iconLoader()->loadIcon(d->iconName, KIconLoader::Desktop,
                                         qMin(size.width(), size.height()),
                                         kstate, d->overlays);
    return pix.size();
}

void KIconEngine::paint( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)

    const int kstate = qIconModeToKIconState(mode);
    KIconLoader::Group group = KIconLoader::Desktop;

    if (QWidget* targetWidget = dynamic_cast<QWidget*>(painter->device())) {
        if (qobject_cast<QMenu*>(targetWidget))
            group = KIconLoader::Small;
        else if (qobject_cast<QToolBar*>(targetWidget->parent()))
            group = KIconLoader::Toolbar;
    }

    const int iconSize = qMin(rect.width(), rect.height());
    const QPixmap pix = iconLoader()->loadIcon(d->iconName, group, iconSize, kstate, d->overlays);
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

    const int kstate = qIconModeToKIconState(mode);
    const int iconSize = qMin(size.width(), size.height());

    painter.drawPixmap(QPoint(), iconLoader()->loadIcon(d->iconName, KIconLoader::Desktop,
                                                        iconSize, kstate, d->overlays));

    return pix;
}

QString KIconEngine::key() const
{
    return QLatin1String("KIconEngine");
}

QIconEngineV2 *KIconEngine::clone() const
{
    return new KIconEngine(d->iconName, d->iconLoader, d->overlays);
}

bool KIconEngine::read(QDataStream &in)
{
    in >> d->iconName >> d->overlays;
    return true;
}

bool KIconEngine::write(QDataStream &out) const
{
    out << d->iconName << d->overlays;
    return true;
}


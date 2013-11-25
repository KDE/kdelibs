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

#include <kiconloader.h>

#include <QPainter>


KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader, const QStringList& overlays)
    : mIconName(iconName),
      mOverlays(overlays),
      mIconLoader(iconLoader)
{
}

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader)
    : mIconName(iconName),
      mIconLoader(iconLoader)
{
}

static inline int qIconModeToKIconState( QIcon::Mode mode )
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
    Q_UNUSED(mode)
    const int iconSize = qMin(size.width(), size.height());
    return QSize(iconSize, iconSize);
}

void KIconEngine::paint(QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state)
{
    if (!mIconLoader) {
        return;
    }

    Q_UNUSED(state)

    const int kstate = qIconModeToKIconState(mode);
    const int iconSize = qMin(rect.width(), rect.height());
    const QPixmap pix = mIconLoader.data()->loadIcon(mIconName, KIconLoader::Desktop, iconSize, kstate, mOverlays);
    painter->drawPixmap(rect, pix);
}

QPixmap KIconEngine::pixmap(const QSize & size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state)

    if (!mIconLoader) {
        QPixmap pm(size);
        pm.fill(Qt::transparent);
        return pm;
    }

    const int kstate = qIconModeToKIconState(mode);
    const int iconSize = qMin(size.width(), size.height());
    QPixmap pix = mIconLoader.data()->loadIcon(mIconName, KIconLoader::Desktop, iconSize, kstate, mOverlays);

    if (pix.size() == size) {
        return pix;
    }

    QPixmap pix2(size);
    pix2.fill(QColor(0,0,0,0));

    QPainter painter(&pix2);
    painter.drawPixmap(QPoint(), pix);

    return pix2;
}

QString KIconEngine::iconName() const
{
    return mIconName;
}

QList<QSize> KIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
    Q_UNUSED(mode);
    Q_UNUSED(state);
    return QList<QSize>() << QSize(16, 16)
                          << QSize(22, 22)
                          << QSize(32, 32)
                          << QSize(48, 48)
                          << QSize(64, 64)
                          << QSize(128, 128)
                          << QSize(256, 256);
}

QString KIconEngine::key() const
{
    return QString::fromLatin1("KIconEngine");
}

QIconEngine *KIconEngine::clone() const
{
    return new KIconEngine(mIconName, mIconLoader.data(), mOverlays);
}

bool KIconEngine::read(QDataStream &in)
{
    in >> mIconName >> mOverlays;
    return true;
}

bool KIconEngine::write(QDataStream &out) const
{
    out << mIconName << mOverlays;
    return true;
}

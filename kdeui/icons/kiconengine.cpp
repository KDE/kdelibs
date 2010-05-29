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

#include <kiconloader.h>

#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QApplication>


KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader, const QStringList& overlays)
{
    mIconName = iconName;
    mIconLoader = iconLoader;
    mOverlays = overlays;
}

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader)
{
    mIconName = iconName;
    mIconLoader = iconLoader;
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
    const QPixmap pix = mIconLoader->loadIcon(mIconName, group, iconSize, kstate, mOverlays);
    painter->drawPixmap(rect, pix);
}

QPixmap KIconEngine::pixmap( const QSize & size, QIcon::Mode mode, QIcon::State state )
{
    Q_UNUSED(state)

    const int kstate = qIconModeToKIconState(mode);
    const int iconSize = qMin(size.width(), size.height());
    QPixmap pix = mIconLoader->loadIcon(mIconName, KIconLoader::Desktop, iconSize, kstate, mOverlays);

    if(pix.size() == size)
        return pix;

    QPixmap pix2(size);
    pix2.fill(QColor(0,0,0,0));

    QPainter painter(&pix2);
    painter.drawPixmap(QPoint(), pix);

    return pix2;
}

QString KIconEngine::key() const
{
    return QString::fromLatin1("KIconEngine");
}

QIconEngineV2 *KIconEngine::clone() const
{
    return new KIconEngine(mIconName, mIconLoader, mOverlays);
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

// FIXME: QIconEngineV2::IconNameHook has been introduced in Qt 4.7.
// Remove next line when we depend on Qt 4.7.
#define IconNameHook 2
void KIconEngine::virtual_hook(int id, void *data)
{
    switch (id) {
    case IconNameHook: {
        QString *name = reinterpret_cast<QString*>(data);
        *name = mIconName;
        break;
    }
    default:
        QIconEngineV2::virtual_hook(id, data);
        break;
    }
}

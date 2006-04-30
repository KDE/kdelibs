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
    KIconLoader* iconLoader;
};

KIconEngine::KIconEngine(const QString& iconName, KIconLoader* iconLoader)
  : d(new KIconEnginePrivate)
{
  d->iconName = iconName;
  d->iconLoader = iconLoader;
}

KIconEngine::~KIconEngine()
{
  delete d;
}

QSize KIconEngine::actualSize( const QSize & size, QIcon::Mode mode, QIcon::State state )
{
  Q_UNUSED(state)

  K3Icon::States kstate;
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

  QPixmap pix = iconLoader()->loadIcon(d->iconName, K3Icon::Desktop, qMin(size.width(), size.height()), kstate);
  return pix.size();
}

void KIconEngine::paint( QPainter * painter, const QRect & rect, QIcon::Mode mode, QIcon::State state )
{
  Q_UNUSED(state)

  K3Icon::States kstate;
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
  {
    QPainter painter(&pix);

    K3Icon::States kstate;
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

    painter.fillRect(QRect(QPoint(), size), state == QIcon::Off ?
                     QApplication::palette().color( QPalette::Background ) :
                     QApplication::palette().color( QPalette::Highlight ) );
    painter.drawPixmap(QPoint(), iconLoader()->loadIcon(d->iconName, K3Icon::Desktop,
                                                        qMin(size.width(), size.height()), kstate));
  }

  return pix;
}

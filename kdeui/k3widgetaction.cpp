/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "k3widgetaction.h"

#include <QToolBar>
#include <QEvent>

#include <kdebug.h>
#include <klocale.h>

K3WidgetAction::K3WidgetAction( QWidget* widget,
    const QString& text, KActionCollection* parent, const QString& name )
  : KAction( text, parent, name )
  , m_widget( widget )
  , d(0L)
{
  setDefaultWidget(widget);
}

K3WidgetAction::K3WidgetAction( QWidget* widget,
    const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const QString& name )
  : KAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , d(0L)
{
  setDefaultWidget(widget);
}

K3WidgetAction::~K3WidgetAction()
{
  //delete d;
}

bool K3WidgetAction::event(QEvent* event)
{
  if (event->type() == QEvent::ActionChanged) {
    if (isEnabled() != m_widget->isEnabled())
      m_widget->setEnabled(isEnabled());
  }
  
  return KAction::event(event);
}

/* vim: et sw=2 ts=2
 */

#include "k3widgetaction.moc"

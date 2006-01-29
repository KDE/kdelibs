/*
 *  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Matthias Ettrich <ettrich@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include <QMenu>
#include <QMouseEvent>
#include <QWidget>

#include "kcmenumngr.h"
#include "kconfig.h"
#include "kglobal.h"
#include "kshortcut.h"

#undef KeyPress
#undef None

KContextMenuManager* KContextMenuManager::mSelf = 0;

KContextMenuManager::KContextMenuManager( QObject* parent )
    : QObject( parent )
{
  KConfigGroup shortCutGroup( KGlobal::config(), QLatin1String("Shortcuts") );
  mMenuKey = KShortcut( shortCutGroup.readEntry(QLatin1String("PopupContextMenu"), QString::fromLatin1("Menu") ) ).keyCodeQt();

  KConfigGroup contextMenuGroup( &shortCutGroup, QLatin1String("ContextMenus") );
  mShowOnPress = contextMenuGroup.readEntry(QLatin1String("ShowOnPress"), true);
}

KContextMenuManager::~KContextMenuManager()
{
}

bool KContextMenuManager::showOnButtonPress()
{
  if ( !mSelf )
    mSelf = new KContextMenuManager;

  return mSelf->mShowOnPress;
}


void KContextMenuManager::insert( QWidget* widget, QMenu* popup )
{
  if ( !mSelf )
    mSelf = new KContextMenuManager;

  mSelf->connect( widget, SIGNAL( destroyed() ), mSelf, SLOT( widgetDestroyed() ) );
  mSelf->mMenus.insert( widget, popup );
  widget->installEventFilter( mSelf );
}

bool KContextMenuManager::eventFilter( QObject *o, QEvent *e )
{
  QMenu* popup = 0;
  QPoint pos;

  switch ( e->type() ) {
    case QEvent::MouseButtonPress:
      if (((QMouseEvent*) e )->button() != Qt::RightButton )
        break;

      if ( !mShowOnPress )
        return true; // eat event for safety

      popup = mMenus[o];
      pos = ((QMouseEvent*) e )->globalPos();
      break;
    case QEvent::MouseButtonRelease:
      if ( mShowOnPress  || ((QMouseEvent*) e )->button() != Qt::RightButton )
        break;

      popup = mMenus[o];
      pos = ((QMouseEvent*) e )->globalPos();
      break;
    case QEvent::KeyPress:
      {
        if ( !o->isWidgetType() )
          break;

        QKeyEvent *k = (QKeyEvent *)e;
        int key = k->key();

        if ( k->modifiers() & Qt::ShiftModifier )
          key |= Qt::SHIFT;

        if ( k->modifiers() & Qt::ControlModifier )
          key |= Qt::CTRL;

        if ( k->modifiers() & Qt::AltModifier )
          key |= Qt::ALT;

        if ( key != mMenuKey )
          break;

        popup = mMenus[o];

        if ( popup ) {
          QWidget* w = (QWidget*) o ;

          // ### workaround
          pos = w->mapToGlobal( w->rect().center() );
          // with later Qt snapshot
          // pos = w->mapToGlobal( w->microFocusHint().center() );
        }
      }
      break;
    default:
      break;
  }

  if ( popup ) {
    popup->popup( pos );
    return true;
  }

  return false;
}

void KContextMenuManager::widgetDestroyed()
{
  if ( mMenus.find( (QObject*)sender() ) )
    mMenus.remove( (QObject*)sender() );
}

#include "kcmenumngr.moc"

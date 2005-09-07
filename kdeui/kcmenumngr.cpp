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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#include <qwidget.h>
#include <q3popupmenu.h>
#include "kcmenumngr.h"
#include "kglobal.h"
#include "kconfig.h"
#include "kshortcut.h"
#include <QMouseEvent>

#undef KeyPress
#undef None

template class Q3PtrDict<Q3PopupMenu>;

KContextMenuManager* KContextMenuManager::manager = 0;

KContextMenuManager::KContextMenuManager( QObject* parent )
    : QObject( parent )
{
    KConfigGroupSaver saver ( KGlobal::config(), QString::fromLatin1("Shortcuts") ) ;
    menuKey = KShortcut( saver.config()->readEntry(QString::fromLatin1("PopupContextMenu"), QString::fromLatin1("Menu") ) ).keyCodeQt();
    saver.config()->setGroup( QString::fromLatin1("ContextMenus") ) ;
    showOnPress = saver.config()->readBoolEntry(QString::fromLatin1("ShowOnPress"), true );
}

KContextMenuManager::~KContextMenuManager()
{
}


bool KContextMenuManager::showOnButtonPress( void )
{
  if ( !manager )
	manager = new KContextMenuManager;
  return manager->showOnPress;
}


void KContextMenuManager::insert( QWidget* widget, Q3PopupMenu* popup )
{
    if ( !manager )
	manager = new KContextMenuManager;

    manager->connect( widget, SIGNAL( destroyed() ), manager, SLOT( widgetDestroyed() ) );
    manager->menus.insert( widget, popup );
    widget->installEventFilter( manager );
}

bool KContextMenuManager::eventFilter( QObject *o, QEvent * e)
{
    Q3PopupMenu* popup = 0;
    QPoint pos;
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
	if (((QMouseEvent*) e )->button() != Qt::RightButton )
	    break;
	if ( !showOnPress )
	    return true; // eat event for safety
	popup = menus[o];
	pos = ((QMouseEvent*) e )->globalPos();
	break;
    case QEvent::MouseButtonRelease:
	if ( showOnPress  || ((QMouseEvent*) e )->button() != Qt::RightButton )
	    break;
	popup = menus[o];
	pos = ((QMouseEvent*) e )->globalPos();
	break;
    case QEvent::KeyPress:
	{
	    if ( !o->isWidgetType() )
		break;
	    QKeyEvent *k = (QKeyEvent *)e;
	    int key = k->key();
	    if ( k->state() & Qt::ShiftModifier )
		key |= Qt::SHIFT;
	    if ( k->state() & Qt::ControlModifier )
		key |= Qt::CTRL;
	    if ( k->state() & Qt::AltModifier )
		key |= Qt::ALT;
	    if ( key != menuKey )
		break;
	    popup = menus[o];
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
    if ( menus.find( (QObject*)sender() ) )
	menus.remove( (QObject*)sender() );
}

#include "kcmenumngr.moc"

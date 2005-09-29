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
#include <qwidget.h>
#include <qpopupmenu.h>
#include "kcmenumngr.h"
#include "kglobal.h"
#include "kconfig.h"
#include "kshortcut.h"

#undef KeyPress
#undef None

template class QPtrDict<QPopupMenu>;

KContextMenuManager* KContextMenuManager::manager = 0;

KContextMenuManager::KContextMenuManager( QObject* parent, const char* name )
    : QObject( parent, name)
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


void KContextMenuManager::insert( QWidget* widget, QPopupMenu* popup )
{
    if ( !manager )
	manager = new KContextMenuManager;
    
    manager->connect( widget, SIGNAL( destroyed() ), manager, SLOT( widgetDestroyed() ) );
    manager->menus.insert( widget, popup );
    widget->installEventFilter( manager );
}

bool KContextMenuManager::eventFilter( QObject *o, QEvent * e)
{
    QPopupMenu* popup = 0;
    QPoint pos;
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
	if (((QMouseEvent*) e )->button() != RightButton )
	    break;
	if ( !showOnPress )
	    return true; // eat event for safety
	popup = menus[o];
	pos = ((QMouseEvent*) e )->globalPos();
	break;
    case QEvent::MouseButtonRelease:
	if ( showOnPress  || ((QMouseEvent*) e )->button() != RightButton )
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
	    if ( k->state() & ShiftButton )
		key |= SHIFT;
	    if ( k->state() & ControlButton )
		key |= CTRL;
	    if ( k->state() & AltButton )
		key |= ALT;
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

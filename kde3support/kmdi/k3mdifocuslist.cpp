/* This file is part of the KDE project
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>

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

#include "k3mdifocuslist.h"
#include "k3mdifocuslist.moc"
#include <qobject.h>
#include <kdebug.h>

K3MdiFocusList::K3MdiFocusList( QObject *parent ) : QObject( parent )
{}

K3MdiFocusList::~K3MdiFocusList()
{}

void K3MdiFocusList::addWidgetTree( QWidget* w )
{
	//this method should never be called twice on the same hierarchy
	m_list.insert( w, w->focusPolicy() );
	w->setFocusPolicy( Qt::ClickFocus );
	kDebug( 760 ) << "K3MdiFocusList::addWidgetTree: adding toplevel" << endl;
	connect( w, SIGNAL( destroyed( QObject * ) ), this, SLOT( objectHasBeenDestroyed( QObject* ) ) );
	
	QList<QWidget*> l = w->findChildren<QWidget *>();
	foreach ( QWidget *wid, l ) {
		m_list.insert( wid, wid->focusPolicy() );
		wid->setFocusPolicy( Qt::ClickFocus );
		kDebug( 760 ) << "K3MdiFocusList::addWidgetTree: adding widget" << endl;
		connect( wid, SIGNAL( destroyed( QObject * ) ), this, SLOT( objectHasBeenDestroyed( QObject* ) ) );
	}
}

void K3MdiFocusList::restore()
{
	for ( QMap<QWidget*, Qt::FocusPolicy>::const_iterator it = m_list.constBegin();it != m_list.constEnd();++it )
	{
		it.key() ->setFocusPolicy( it.data() );
	}
	m_list.clear();
}


void K3MdiFocusList::objectHasBeenDestroyed( QObject * o )
{
	if ( !o || !o->isWidgetType() )
		return ;
	QWidget *w = ( QWidget* ) o;
	m_list.remove( w );
}

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kglobalsettings.h>
#include <qdragobject.h>

#include "kpushbutton.h"

KPushButton::KPushButton( QWidget *parent, const char *name )
    : QPushButton( parent, name ),
      m_dragEnabled( false )
{
}

KPushButton::KPushButton( const QString &text, QWidget *parent,
				  const char *name)
    : QPushButton( text, parent, name ),
      m_dragEnabled( false )
{
}

KPushButton::KPushButton( const QIconSet &icon, const QString &text,
				  QWidget *parent, const char *name )
    : QPushButton( icon, text, parent, name ),
      m_dragEnabled( false )
{
}

KPushButton::~KPushButton()
{
}


void KPushButton::setDragEnabled( bool enable )
{
    m_dragEnabled = enable;
}

void KPushButton::mousePressEvent( QMouseEvent *e )
{
    if ( m_dragEnabled )
	startPos = e->pos();
    QPushButton::mousePressEvent( e );
}

void KPushButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !m_dragEnabled ) {
	QPushButton::mouseMoveEvent( e );
	return;
    }

    if ( (e->state() & LeftButton) &&
	 (e->pos() - startPos).manhattanLength() >
	 KGlobalSettings::dndEventDelay() ) {

	startDrag();
	setDown( false );
    }
}

QDragObject * KPushButton::dragObject()
{
    return 0L;
}

void KPushButton::startDrag()
{
    QDragObject *d = dragObject();
    if ( d )
	d->dragCopy();
}

#include "kpushbutton.moc"

/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

    This library is free software; you can redistribute it and/or
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpainter.h>
#include <qdrawutil.h>
#include "kcolordlg.h"
#include "kcolorbtn.h"
#include "kcolordrag.h"

#include "kcolorbtn.h"

KColorButton::KColorButton( QWidget *parent, const char *name )
	: QPushButton( parent, name )
{
	connect( this, SIGNAL( clicked() ), SLOT( slotClicked() ) );
	setAcceptDrops( true);
}

KColorButton::KColorButton( const QColor &c, QWidget *parent, const char *name )
	: QPushButton( parent, name )
{
	connect( this, SIGNAL( clicked() ), SLOT( slotClicked() ) );
	col = c;
	setAcceptDrops( true);
}

void KColorButton::setColor( const QColor &c )
{
	col = c;
	repaint( false );
}

void KColorButton::slotClicked()
{
	if ( KColorDialog::getColor( col ) == QDialog::Rejected )
		return;

	repaint( false );

	emit changed( col );
}

void KColorButton::drawButtonLabel( QPainter *painter )
{
	int w = ( style() == WindowsStyle ) ? 11 : 10;
	
	QColor lnCol = colorGroup().text();
	QColor fillCol = isEnabled() ? col : backgroundColor();

	if ( style() == WindowsStyle && isDown() )
	{
		qDrawPlainRect( painter, w/2+1, w/2+1, width()-w, height()-w,
		    lnCol, 1, 0 );
		w += 2;
		painter->fillRect( w/2+1, w/2+1, width()-w, height()-w, fillCol );
	}
	else
	{
		qDrawPlainRect( painter, w/2, w/2, width()-w, height()-w,
		    lnCol, 1, 0 );
		w += 2;
		painter->fillRect( w/2, w/2, width() - w, height() - w, fillCol );
	}
		
}

void KColorButton::dragEnterEvent( QDragEnterEvent *event)
{
        event->accept( KColorDrag::canDecode( event));
}

void KColorButton::dropEvent( QDropEvent *event)
{
        QColor c;
        if( KColorDrag::decode( event, c)) {
            setColor(c);
	    emit changed( c);
        }
}

#include "kcolorbtn.moc"

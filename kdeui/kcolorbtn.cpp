/*  This file is part of the KDE libraries
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
        inMouse = false;
}

KColorButton::KColorButton( const QColor &c, QWidget *parent, const char *name )
	: QPushButton( parent, name )
{
	connect( this, SIGNAL( clicked() ), SLOT( slotClicked() ) );
	col = c;
        setAcceptDrops( true);
        inMouse = false;
}

void KColorButton::setColor( const QColor &c )
{
	col = c;
	repaint( false );
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
        event->accept( KColorDrag::canDecode( event) && isEnabled());
}

void KColorButton::dropEvent( QDropEvent *event)
{
        QColor c;
        if( KColorDrag::decode( event, c)) {
            setColor(c);
	    emit changed( c);
        }
}

void KColorButton::mousePressEvent( QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    if(!e->state() && LeftButton)
        return;
    dragged = false;

}

void KColorButton::mouseMoveEvent( QMouseEvent *e)
{
	// Call parent's handler
        // It seems that this doesn't release the button??

        // I moved this around a little (mosfet)
        // QPushButton::mouseMoveEvent( e);

        if(!inMouse){
            mPos = e->pos();
            inMouse = true;
            return;
        }
        if(e->x() > mPos.x()+5 || e->x() < mPos.x()-5 ||
           e->y() > mPos.y()+5 || e->y() < mPos.y()-5){
            // Drag color object
            dragged = true;
            KColorDrag *d = KColorDrag::makeDrag( color(), this);
            d->dragCopy();
            // Fake a release event for QPushButton (mosfet)
            QMouseEvent evTmp(QEvent::MouseButtonRelease,
                              e->pos(), e->globalPos(),
                              QMouseEvent::LeftButton,
                              QMouseEvent::LeftButton);
            mouseReleaseEvent(&evTmp);
        }
}

void KColorButton::mouseReleaseEvent( QMouseEvent *e )
{
    QPushButton::mouseReleaseEvent(e);
    inMouse = false;
    if(!dragged){
        if (KColorDialog::getColor( col ) == QDialog::Rejected )
            return;
        repaint( false );
        emit changed( col );
    }
    dragged = false; // no idea why this is needed...
}

#include "kcolorbtn.moc"

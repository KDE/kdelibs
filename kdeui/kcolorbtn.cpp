/*  This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
    Copyright (C) 1999 Cristian Tibirna (ctibirna@kde.org)

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
#include <kglobal.h>
#include "kcolordlg.h"
#include "kcolorbtn.h"
#include "kcolordrag.h"

KColorButton::KColorButton( QWidget *parent, const char *name )
  : QPushButton( parent, name ), dragFlag(false)
{
  setAcceptDrops( true);
}

KColorButton::KColorButton( const QColor &c, QWidget *parent,
			    const char *name )
  : QPushButton( parent, name ), col(c), dragFlag(false)
{
  setAcceptDrops( true);
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
  
  if ( style() == WindowsStyle && isDown() ) {
    qDrawPlainRect( painter, w/2+1, w/2+1, width()-w, height()-w,
		    lnCol, 1, 0 );
    w += 2;
    painter->fillRect( w/2+1, w/2+1, width()-w, height()-w, fillCol );
  } else {
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
  mPos = e->pos();
  QPushButton::mousePressEvent(e);
}

void KColorButton::mouseMoveEvent( QMouseEvent *e)
{
  int delay = KGlobal::dndEventDelay();

  if(e->x() >= mPos.x()+delay || e->x() <= mPos.x()-delay ||
     e->y() >= mPos.y()+delay || e->y() <= mPos.y()-delay) {
    // Drag color object
    KColorDrag *d = KColorDrag::makeDrag( color(), this);
    d->dragCopy();
    dragFlag = true;
    // Fake a release event for QPushButton (mosfet)
    QMouseEvent evTmp(QEvent::MouseButtonRelease,
		      e->pos(), e->globalPos(),
    		      QMouseEvent::LeftButton,
    		      QMouseEvent::LeftButton);
    mouseReleaseEvent(&evTmp);
    dragFlag = false;
  }
}

void KColorButton::mouseReleaseEvent( QMouseEvent *e )
{
  QPushButton::mouseReleaseEvent(e);
  if (!dragFlag) {
    if (KColorDialog::getColor( col, this ) == QDialog::Rejected )
      return;
    repaint( false );
    emit changed( col );
  }
}

#include "kcolorbtn.moc"

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

#include <config.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qstyle.h>
#include <kglobalsettings.h>
#include "kcolordialog.h"
#include "kcolorbutton.h"
#include "kcolordrag.h"

KColorButton::KColorButton( QWidget *parent, const char *name )
  : QPushButton( parent, name )
{
  setAcceptDrops( true);
  
  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

KColorButton::KColorButton( const QColor &c, QWidget *parent,
			    const char *name )
  : QPushButton( parent, name ), col(c)
{
  setAcceptDrops( true);

  // 2000-10-15 (putzer): fixes broken keyboard usage
  connect (this, SIGNAL(clicked()), this, SLOT(chooseColor()));
}

void KColorButton::setColor( const QColor &c )
{
  col = c;
  repaint( false );
}

void KColorButton::drawButtonLabel( QPainter *painter )
{
#if QT_VERSION < 300
  QRect r = style().buttonRect( 0, 0, width(), height() );
#else
  // ### FIXME: correct??
  QRect r = style().subRect( QStyle::SR_PushButtonContents, this );
#endif
  int l = r.x();
  int t = r.y();
  int w = r.width();
  int h = r.height();
  int b = 5;
  
  QColor lnCol = colorGroup().text();
  QColor fillCol = isEnabled() ? col : backgroundColor();
  
  if ( isDown() ) {
    qDrawPlainRect( painter, l+b+1, t+b+1, w-b*2, h-b*2, lnCol, 1, 0 );
    b++;
    if ( fillCol.isValid() )
      painter->fillRect( l+b+1, t+b+1, w-b*2, h-b*2, fillCol );
  } else {
    qDrawPlainRect( painter, l+b, t+b, w-b*2, h-b*2, lnCol, 1, 0 );
    b++;
    if ( fillCol.isValid() )
      painter->fillRect( l+b, t+b, w-b*2, h-b*2, fillCol );
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
  if( (e->state() & LeftButton) && 
    (e->pos()-mPos).manhattanLength() > KGlobalSettings::dndEventDelay() )
  {
    // Drag color object
    KColorDrag *d = KColorDrag::makeDrag( color(), this);
    d->dragCopy();
    setDown(false);
  }
}

void KColorButton::chooseColor()
{
  if( KColorDialog::getColor( col, this ) == QDialog::Rejected )
  {
    return;
  }

  repaint( false );
  emit changed( col );
}

#include "kcolorbutton.moc"

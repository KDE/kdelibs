/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
	              1997 Matthias Ettrich (ettrich@kde.org)

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
#include "kbutton.h"

#include <qpainter.h>
#include <qdrawutil.h>

KButton::KButton( QWidget *_parent, const char *name )
    : QButton( _parent , name)
{
  raised = FALSE;
  setFocusPolicy( NoFocus );
}

void KButton::enterEvent( QEvent* )
{
  if ( isEnabled() )
    {
      raised = TRUE;
      repaint(FALSE);
    }
}

void KButton::leaveEvent( QEvent * )
{
  if( raised != FALSE )
    {
      raised = FALSE;
      repaint();
    }
}
    
void KButton::drawButton( QPainter *_painter )
{
  paint( _painter );
}

void KButton::drawButtonLabel( QPainter *_painter )
{
  paint( _painter );
}

void KButton::paint( QPainter *painter )
{
  if ( isDown() || isOn() )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), 
			height(), colorGroup(), TRUE );
      else
	qDrawShadePanel( painter, 0, 0, width(), 
			 height(), colorGroup(), TRUE, 2, 0L );
    }
  else if ( raised )
    {
      if ( style() == WindowsStyle )
	qDrawWinButton( painter, 0, 0, width(), height(), 
			colorGroup(), FALSE );
      else
	qDrawShadePanel( painter, 0, 0, width(), height(), 
			 colorGroup(), FALSE, 2, 0L );
    }
  
  if ( pixmap() )
    {
      int dx = ( width() - pixmap()->width() ) / 2;
      int dy = ( height() - pixmap()->height() ) / 2;
      if ( isDown() && style() == WindowsStyle ) {
       ++dx;
       ++dy;
      }
      painter->drawPixmap( dx, dy, *pixmap() );
    }
}


KButton::~KButton()
{
}

#include "kbutton.h"

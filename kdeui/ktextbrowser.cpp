/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999 Espen Sand (espensa@online.no)
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
 */

/*
****************************************************************************
*
* $Log:
*
****************************************************************************
*/


#include <kapp.h>
#include <ktextbrowser.h>

KTextBrowser::KTextBrowser( QWidget *parent, const char *name, 
			    bool notifyClick )
  : QTextBrowser( parent, name ), mNotifyClick(notifyClick)
{
  connect( this, SIGNAL(highlighted(const QString &)),
	   this, SLOT(refChanged(const QString &)));
}

KTextBrowser::~KTextBrowser( void )
{
}


void KTextBrowser::setNotifyClick( bool notifyClick )
{
  mNotifyClick = notifyClick;
}


void KTextBrowser::refChanged( const QString &ref )
{
  mActiveRef = ref;
}


void KTextBrowser::viewportMouseReleaseEvent( QMouseEvent * )
{
  if( mActiveRef.isNull() == true )
  {
    return;
  }

  if( mActiveRef.contains('@') == true )
  {
    if( mNotifyClick == false )
    {
      kapp->invokeMailer( mActiveRef, QString::null );
    }
    else
    {
      emit mailClick( QString::null, mActiveRef );
    }
  }
  else
  {
    if( mNotifyClick == false )
    {
      kapp->invokeBrowser( mActiveRef );
    }
    else
    {
      emit urlClick( mActiveRef );
    }
  }
}


void KTextBrowser::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Key_Escape )
  {
    e->ignore();
  }
  else
  {
    QTextBrowser::keyPressEvent(e);
  }
}





#include "ktextbrowser.moc"









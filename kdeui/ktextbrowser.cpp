/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
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
* $Log$
* Revision 1.6  2000/01/03 18:48:57  espen
* The widget will ignore a key sequence
* containing F1. Since this widget is used
* in dialogs (eg KDialogBase), F1 and Shift+F1
* can be used to start the help operation.
*
*
****************************************************************************
*/


#include <kapp.h>
#include <ktextbrowser.h>
#include <kcursor.h>

KTextBrowser::KTextBrowser( QWidget *parent, const char *name,
			    bool notifyClick )
  : QTextBrowser( parent, name ), mNotifyClick(notifyClick)
{
  //
  //1999-10-04 Espen Sand: Not required anymore ?
  //connect( this, SIGNAL(highlighted(const QString &)),
  //   this, SLOT(refChanged(const QString &)));
}

KTextBrowser::~KTextBrowser( void )
{
}


void KTextBrowser::setNotifyClick( bool notifyClick )
{
  mNotifyClick = notifyClick;
}


void KTextBrowser::setSource( const QString& name )
{
  if( name.isNull() == true )
  {
    return;
  }

  if( name.contains('@') == true )
  {
    if( mNotifyClick == false )
    {
      kapp->invokeMailer( name, QString::null );
    }
    else
    {
      emit mailClick( QString::null, name );
    }
  }
  else
  {
    if( mNotifyClick == false )
    {
      kapp->invokeBrowser( name );
    }
    else
    {
      emit urlClick( name );
    }
  }
}


void KTextBrowser::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Key_F1 )
  {
    e->ignore();
  }
  else
  {
    QTextBrowser::keyPressEvent(e);
  }
}

void KTextBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
  // do this first so we get the right type of cursor
  QTextBrowser::viewportMouseMoveEvent(e);

  if ( viewport()->cursor().shape() == PointingHandCursor )
    viewport()->setCursor( KCursor::handCursor() );
}

#include "ktextbrowser.moc"









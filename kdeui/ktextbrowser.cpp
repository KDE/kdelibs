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
* $Log
*
****************************************************************************
*/


#include <ktextbrowser.h>

KTextBrowser::KTextBrowser( QWidget *parent, const char *name )
  : QTextBrowser( parent, name )
{
  connect( this, SIGNAL(highlighted(const QString &)),
	   this, SLOT(refChanged(const QString &)));
}

KTextBrowser::~KTextBrowser( void )
{
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
    emit mailClick( QString::null, mActiveRef );
  }
  else
  {
    emit urlClick( mActiveRef );
  }
}

#include "ktextbrowser.moc"

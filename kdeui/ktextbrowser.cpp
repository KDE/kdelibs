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
* Revision 1.11  2002/03/04 00:51:51  lunakl
* Keep BC changes (the patch is almost 100KiB of boring stuff
* ... anybody willing to review? ;) ).
*
* Revision 1.10  2001/11/24 01:19:04  pfeiffer
* make the wheelmouse work the same way as in KDE2, i.e. Ctrl-Wheel
* scrolls faster, instead of zooming. Do we need to make this configurable?
* I for one find myself scrolling much more often than zooming.
*
* Revision 1.9  2001/10/10 17:40:39  mueller
* CVS_SILENT: fixincludes
*
* Revision 1.8  2000/06/01 09:36:12  gehrmab
* We like orthogonal APIs
*
* Revision 1.7  2000/05/26 04:20:33  granroth
* Use the KCursor::handCursor instead of the Qt hand cursor with
* KTextBrowser.  This is for consistency.
*
* There is a little bit of flicker when it switches to the hand, though.
* That's because it first switches to the Qt version in the
* QTextBrowser code and I use that in the KTextBrowser version to know
* when to switch to the hand.  I couldn't figure out any other way to do
* this without completely reimplementing all of the enter events -- what
* a pain.
*
* Revision 1.6  2000/01/03 18:48:57  espen
* The widget will ignore a key sequence
* containing F1. Since this widget is used
* in dialogs (eg KDialogBase), F1 and Shift+F1
* can be used to start the help operation.
*
*
****************************************************************************
*/


#include <kapplication.h>
#include <kglobalsettings.h>
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


bool KTextBrowser::isNotifyClick() const
{
  return mNotifyClick;
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

void KTextBrowser::contentsWheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        QTextBrowser::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        QScrollView::contentsWheelEvent( e );
}

void KTextBrowser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextbrowser.moc"

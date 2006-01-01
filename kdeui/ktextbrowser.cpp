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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */


#include <q3popupmenu.h>
#include <qevent.h>
#include <ktoolinvocation.h>
#include <kglobalsettings.h>
#include <ktextbrowser.h>
#include <kcursor.h>
#include <kurl.h>
#include <kiconloader.h>

KTextBrowser::KTextBrowser( QWidget *parent, const char *name,
			    bool notifyClick )
  : Q3TextBrowser( parent, name ), mNotifyClick(notifyClick)
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
  if( name.isNull() )
  {
    return;
  }

  if( name.find('@') > -1 )
  {
    if( !mNotifyClick )
    {
      KToolInvocation::invokeMailer( KURL( name ) );
    }
    else
    {
      emit mailClick( QString(), name );
    }
  }
  else
  {
    if( !mNotifyClick )
    {
      KToolInvocation::invokeBrowser( name );
    }
    else
    {
      emit urlClick( name );
    }
  }
}


void KTextBrowser::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Qt::Key_F1 )
  {
    e->ignore();
  }
  else
  {
    Q3TextBrowser::keyPressEvent(e);
  }
}

void KTextBrowser::viewportMouseMoveEvent( QMouseEvent* e)
{
  // do this first so we get the right type of cursor
  Q3TextBrowser::viewportMouseMoveEvent(e);

  if ( viewport()->cursor().shape() == Qt::PointingHandCursor )
    viewport()->setCursor( KCursor::handCursor() );
}

void KTextBrowser::contentsWheelEvent( QWheelEvent *e )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        Q3TextBrowser::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        Q3ScrollView::contentsWheelEvent( e );
}

Q3PopupMenu *KTextBrowser::createPopupMenu( const QPoint & pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    Q3PopupMenu *popup = Q3TextBrowser::createPopupMenu( pos );

    if ( isReadOnly() )
      popup->changeItem( popup->idAt(0), SmallIconSet("editcopy"), popup->text( popup->idAt(0) ) );
    else {
      int id = popup->idAt(0);
      popup->changeItem( id - IdUndo, SmallIconSet("undo"), popup->text( id - IdUndo) );
      popup->changeItem( id - IdRedo, SmallIconSet("redo"), popup->text( id - IdRedo) );
      popup->changeItem( id - IdCut, SmallIconSet("editcut"), popup->text( id - IdCut) );
      popup->changeItem( id - IdCopy, SmallIconSet("editcopy"), popup->text( id - IdCopy) );
      popup->changeItem( id - IdPaste, SmallIconSet("editpaste"), popup->text( id - IdPaste) );
      popup->changeItem( id - IdClear, SmallIconSet("editclear"), popup->text( id - IdClear) );
    }

    return popup;
}

void KTextBrowser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktextbrowser.moc"

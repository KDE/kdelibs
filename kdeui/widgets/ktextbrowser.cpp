/*  This file is part of the KDE Libraries
 *  Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *  Copyright (C) 2006 Urs Wolfer <uwolfer at fwo.ch>
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

#include "ktextbrowser.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtGui/QTextBrowser>
#include <QtGui/QWhatsThis>

#include <kcursor.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kicontheme.h>
#include <kurl.h>
#include <ktoolinvocation.h>

class KTextBrowser::Private
{
  public:
    Private()
      : notifyClick( false )
    {
    }

    ~Private()
    {
    }

    bool notifyClick;
};

KTextBrowser::KTextBrowser( QWidget *parent, bool notifyClick )
  : QTextBrowser( parent ), d( new Private )
{
  d->notifyClick = notifyClick;
}

KTextBrowser::~KTextBrowser()
{
  delete d;
}


void KTextBrowser::setNotifyClick( bool notifyClick )
{
  d->notifyClick = notifyClick;
}


bool KTextBrowser::isNotifyClick() const
{
  return d->notifyClick;
}


void KTextBrowser::setSource( const QUrl& name )
{
  QString strName = name.toString();
  if ( strName.isNull() )
    return;

  QRegExp whatsthis( "whatsthis:/*([^/].*)" );
  if ( !d->notifyClick && whatsthis.exactMatch( strName ) ) {
     QWhatsThis::showText( QCursor::pos(), whatsthis.cap( 1 ) );
  } else if ( strName.indexOf( '@' ) > -1 ) {
    if ( !d->notifyClick ) {
      KToolInvocation::invokeMailer( KUrl( strName ) );
    } else {
      emit mailClick( QString(), strName );
    }
  } else {
    if ( !d->notifyClick ) {
      KToolInvocation::invokeBrowser( strName );
    } else {
      emit urlClick( strName );
    }
  }
}


void KTextBrowser::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
    event->ignore();
  else if ( event->key() == Qt::Key_F1 )
    event->ignore();
  else
    QTextBrowser::keyPressEvent( event );
}

void KTextBrowser::wheelEvent( QWheelEvent *event )
{
  if ( KGlobalSettings::wheelMouseZooms() )
    QTextBrowser::wheelEvent( event );
  else // thanks, we don't want to zoom, so skip QTextEdit's impl.
    QAbstractScrollArea::wheelEvent( event );
}

void KTextBrowser::contextMenuEvent( QContextMenuEvent *event )
{
  QMenu *popup = createStandardContextMenu(event->pos());
  KIconTheme::assignIconsToContextMenu( isReadOnly() ? KIconTheme::ReadOnlyText
                                                     : KIconTheme::TextEditor,
                                        popup->actions() );

  popup->exec( event->globalPos() );
  delete popup;
}

#include "ktextbrowser.moc"

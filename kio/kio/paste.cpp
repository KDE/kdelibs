/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kio/job.h"
#include "kio/paste.h"
#include "kio/global.h"
#include "kio/netaccess.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qtextstream.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <klocale.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>
#include <ktempfile.h>

bool KIO::isClipboardEmpty()
{
#ifndef QT_NO_MIMECLIPBOARD
  QMimeSource *data = QApplication::clipboard()->data();
  if ( data->provides( "text/uri-list" ) && data->encodedData( "text/uri-list" ).size() > 0 )
    return false;
#else
  // Happens with some versions of Qt Embedded... :/
  // Guess.
  QString data = QApplication::clipboard()->text();
  if(data.contains("://"))
	  return false;
#endif
  return true;
}

KIO::Job *KIO::pasteClipboard( const KURL& dest_url, bool move )
{
  if ( !dest_url.isValid() ) {
    KMessageBox::error( 0L, i18n( "Malformed URL\n%1" ).arg( dest_url.url() ) );
    return 0;
  }

#ifndef QT_NO_MIMECLIPBOARD
  QMimeSource *data = QApplication::clipboard()->data();

  KURL::List urls;
  if ( QUriDrag::canDecode( data ) && KURLDrag::decode( data, urls ) ) {
    if ( urls.count() == 0 ) {
      KMessageBox::error( 0L, i18n("The clipboard is empty"));
      return 0;
    }

    KIO::Job *res = 0;
    if ( move )
      res = KIO::move( urls, dest_url );
    else
      res = KIO::copy( urls, dest_url );

    // If moving, erase the clipboard contents, the original files don't exist anymore
    if ( move )
      QApplication::clipboard()->clear();
    return res;
  }
#else
  QStringList data = QStringList::split("\n", QApplication::clipboard()->text());
  KURL::List urls;
  KURLDrag::decode(data, urls);
#endif

  QByteArray ba;

#ifndef QT_NO_MIMECLIPBOARD
  QString text;
  if ( QTextDrag::canDecode( data ) && QTextDrag::decode( data, text ) )
  {
      QTextStream txtStream( ba, IO_WriteOnly );
      txtStream << text;
  }
  else
      ba = data->encodedData( data->format() );
#else
  QTextStream txtStream( ba, IO_WriteOnly );
  for(QStringList::Iterator it=data.begin(); it!=data.end(); it++)
      txtStream << *it;
#endif

  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return 0;
  }

  pasteData( dest_url, ba );
  return 0;
}

void KIO::pasteData( const KURL& u, const QByteArray& _data )
{
  KLineEditDlg l( i18n("Filename for clipboard content:"), "", 0L );
  int x = l.exec();
  if ( x ) {
    QString url = l.text();
    if ( url.isEmpty() ) {
      KMessageBox::error( 0L, i18n("You did not enter a filename"));
      return;
    }

    KURL myurl(u);
    myurl.addPath( l.text() );

    // We could use KIO::put here, but that would require a class
    // for the slotData call. With NetAcess, we can do a synchronous call.
    // NOTE: upload() overwrites the destination if it exists. TODO dialog box.

    KTempFile tempFile;
    tempFile.setAutoDelete( true );
    tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
    tempFile.close();

    (void) KIO::NetAccess::upload( tempFile.name(), myurl );
  }
}

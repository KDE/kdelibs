/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kio/job.h"
#include "kio/paste.h"
#include "kio/global.h"
#include "kio/netaccess.h"
#include "kio/observer.h"
#include "kio/renamedlg.h"
#include "kio/kprotocolmanager.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qtextstream.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <ktempfile.h>

static KURL getNewFileName( const KURL &u, const QString& text )
{
  bool ok;
  QString dialogText( text );
  if ( dialogText.isEmpty() )
    dialogText = i18n( "Filename for clipboard content:" );
  QString file = KInputDialog::getText( QString::null, dialogText, QString::null, &ok );
  if ( !ok ) 
     return KURL();

  KURL myurl(u);
  myurl.addPath( file );

  if (KIO::NetAccess::exists(myurl, false, 0))
  {
      kdDebug(7007) << "Paste will overwrite file.  Prompting..." << endl;
      KIO::RenameDlg_Result res = KIO::R_OVERWRITE;

      QString newPath;
      // Ask confirmation about resuming previous transfer
      res = Observer::self()->open_RenameDlg(
                          0L, i18n("File Already Exists"),
                          u.prettyURL(0, KURL::StripFileProtocol),
                          myurl.prettyURL(0, KURL::StripFileProtocol),
                          (KIO::RenameDlg_Mode) (KIO::M_OVERWRITE | KIO::M_SINGLE), newPath);

      if ( res == KIO::R_RENAME )
      {
          myurl = newPath;
      }
      else if ( res == KIO::R_CANCEL )
      {
          return KURL();
      }
  }

  return myurl;
}

KIO_EXPORT bool KIO::isClipboardEmpty()
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

KIO_EXPORT KIO::Job *KIO::pasteClipboard( const KURL& dest_url, bool move )
{
  if ( !dest_url.isValid() ) {
    KMessageBox::error( 0L, i18n( "Malformed URL\n%1" ).arg( dest_url.url() ) );
    return 0;
  }

#ifndef QT_NO_MIMECLIPBOARD
  QMimeSource *data = QApplication::clipboard()->data();

  KURL::List urls;
  if ( KURLDrag::canDecode( data ) && KURLDrag::decode( data, urls ) ) {
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

  return pasteDataAsync(dest_url, ba);
}


KIO_EXPORT void KIO::pasteData( const KURL& u, const QByteArray& _data )
{
    KURL new_url = getNewFileName( u, QString::null );
    // We could use KIO::put here, but that would require a class
    // for the slotData call. With NetAccess, we can do a synchronous call.
    
    if (new_url.isEmpty())
       return;
    
    KTempFile tempFile;
    tempFile.setAutoDelete( true );
    tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
    tempFile.close();

    (void) KIO::NetAccess::upload( tempFile.name(), new_url, 0 );
}

KIO_EXPORT KIO::CopyJob* KIO::pasteDataAsync( const KURL& u, const QByteArray& _data )
{
    return pasteDataAsync( u, _data, QString::null );
}

KIO_EXPORT KIO::CopyJob* KIO::pasteDataAsync( const KURL& u, const QByteArray& _data, const QString& text )
{
    KURL new_url = getNewFileName( u, text );

    if (new_url.isEmpty())
       return 0;

     KTempFile tempFile;
     tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
     tempFile.close();

     KURL orig_url;
     orig_url.setPath(tempFile.name());

     return KIO::move( orig_url, new_url );
}

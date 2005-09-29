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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "paste.h"
#include "pastedialog.h"

#include "kio/job.h"
#include "kio/global.h"
#include "kio/netaccess.h"
#include "kio/observer.h"
#include "kio/renamedlg.h"
#include "kio/kprotocolmanager.h"

#include <kurl.h>
#include <kurldrag.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ktempfile.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qtextstream.h>
#include <qvaluevector.h>

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
                          u.pathOrURL(),
                          myurl.pathOrURL(),
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

// The finaly step: write _data to tempfile and move it to neW_url
static KIO::CopyJob* pasteDataAsyncTo( const KURL& new_url, const QByteArray& _data )
{
     KTempFile tempFile;
     tempFile.dataStream()->writeRawBytes( _data.data(), _data.size() );
     tempFile.close();

     KURL orig_url;
     orig_url.setPath(tempFile.name());

     return KIO::move( orig_url, new_url );
}

#ifndef QT_NO_MIMECLIPBOARD
static KIO::CopyJob* chooseAndPaste( const KURL& u, QMimeSource* data,
                                     const QValueVector<QCString>& formats,
                                     const QString& text,
                                     QWidget* widget,
                                     bool clipboard )
{
    QStringList formatLabels;
    for ( uint i = 0; i < formats.size(); ++i ) {
        const QCString& fmt = formats[i];
        KMimeType::Ptr mime = KMimeType::mimeType( fmt );
        if ( mime != KMimeType::defaultMimeTypePtr() )
            formatLabels.append( i18n( "%1 (%2)" ).arg( mime->comment() ).arg( fmt ) );
        else
            formatLabels.append( fmt );
    }

    QString dialogText( text );
    if ( dialogText.isEmpty() )
        dialogText = i18n( "Filename for clipboard content:" );
    KIO::PasteDialog dlg( QString::null, dialogText, QString::null, formatLabels, widget, clipboard );

    if ( dlg.exec() != KDialogBase::Accepted )
        return 0;

    if ( clipboard && dlg.clipboardChanged() ) {
        KMessageBox::sorry( widget,
                            i18n( "The clipboard has changed since you used 'paste': "
                                  "the chosen data format is no longer applicable. "
                                  "Please copy again what you wanted to paste." ) );
        return 0;
    }

    const QString result = dlg.lineEditText();
    const QCString chosenFormat = formats[ dlg.comboItem() ];

    kdDebug() << " result=" << result << " chosenFormat=" << chosenFormat << endl;
    KURL new_url( u );
    new_url.addPath( result );
    // if "data" came from QClipboard, then it was deleted already - by a nice 0-seconds timer
    // In that case, get it again. Let's hope the user didn't copy something else meanwhile :/
    if ( clipboard ) {
        data = QApplication::clipboard()->data();
    }
    const QByteArray ba = data->encodedData( chosenFormat );
    return pasteDataAsyncTo( new_url, ba );
}
#endif

// KDE4: remove
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

#ifndef QT_NO_MIMECLIPBOARD
// The main method for dropping
KIO::CopyJob* KIO::pasteMimeSource( QMimeSource* data, const KURL& dest_url,
                                    const QString& dialogText, QWidget* widget, bool clipboard )
{
  QByteArray ba;

  // Now check for plain text
  // We don't want to display a mimetype choice for a QTextDrag, those mimetypes look ugly.
  QString text;
  if ( QTextDrag::canDecode( data ) && QTextDrag::decode( data, text ) )
  {
      QTextStream txtStream( ba, IO_WriteOnly );
      txtStream << text;
  }
  else
  {
      QValueVector<QCString> formats;
      const char* fmt;
      for ( int i = 0; ( fmt = data->format( i ) ); ++i ) {
          if ( qstrcmp( fmt, "application/x-qiconlist" ) == 0 ) // see QIconDrag
              continue;
          if ( qstrcmp( fmt, "application/x-kde-cutselection" ) == 0 ) // see KonqDrag
              continue;
          if ( strchr( fmt, '/' ) == 0 ) // e.g. TARGETS, MULTIPLE, TIMESTAMP
              continue;
          formats.append( fmt );
      }

      if ( formats.size() > 1 ) {
          return chooseAndPaste( dest_url, data, formats, dialogText, widget, clipboard );
      }
      ba = data->encodedData( formats.first() );
  }
  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return 0;
  }

  return pasteDataAsync( dest_url, ba, dialogText );
}
#endif

// The main method for pasting
KIO_EXPORT KIO::Job *KIO::pasteClipboard( const KURL& dest_url, bool move )
{
  if ( !dest_url.isValid() ) {
    KMessageBox::error( 0L, i18n( "Malformed URL\n%1" ).arg( dest_url.url() ) );
    return 0;
  }

#ifndef QT_NO_MIMECLIPBOARD
  QMimeSource *data = QApplication::clipboard()->data();

  // First check for URLs.
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
  return pasteMimeSource( data, dest_url, QString::null, 0 /*TODO parent widget*/, true /*clipboard*/ );
#else
  QByteArray ba;
  QTextStream txtStream( ba, IO_WriteOnly );
  QStringList data = QStringList::split("\n", QApplication::clipboard()->text());
  KURL::List urls;
  KURLDrag::decode(data, urls);
  QStringList::Iterator end(data.end());
  for(QStringList::Iterator it=data.begin(); it!=end; ++it)
      txtStream << *it;
  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(0, i18n("The clipboard is empty"));
    return 0;
  }
  return pasteDataAsync( dest_url, ba );
#endif
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

    return pasteDataAsyncTo( new_url, _data );
}

KIO_EXPORT QString KIO::pasteActionText()
{
    QMimeSource *data = QApplication::clipboard()->data();
    KURL::List urls;
    if ( KURLDrag::canDecode( data ) && KURLDrag::decode( data, urls ) ) {
        if ( urls.isEmpty() )
            return QString::null; // nothing to paste
        else if ( urls.first().isLocalFile() )
            return i18n( "&Paste File", "&Paste %n Files", urls.count() );
        else
            return i18n( "&Paste URL", "&Paste %n URLs", urls.count() );
    } else if ( data->format(0) != 0 ) {
        return i18n( "&Paste Clipboard Contents" );
    } else {
        return QString::null;
    }
}


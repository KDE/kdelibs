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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#undef QT3_SUPPORT

#include "paste.h"
#include "pastedialog.h"

#include "kio/job.h"
#include "kio/global.h"
#include "kio/netaccess.h"
#include "kio/observer.h"
#include "kio/renamedlg.h"
#include "kio/kprotocolmanager.h"

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ktempfile.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <QMimeData>

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

// The finaly step: write _data to tempfile and move it to newUrl
static KIO::CopyJob* pasteDataAsyncTo( const KURL& newUrl, const QByteArray& _data )
{
     KTempFile tempFile;
     tempFile.dataStream()->writeRawData( _data.data(), _data.size() );
     tempFile.close();

     KURL origUrl;
     origUrl.setPath(tempFile.name());

     return KIO::move( origUrl, newUrl );
}

#ifndef QT_NO_MIMECLIPBOARD
static KIO::CopyJob* chooseAndPaste( const KURL& u, const QMimeData* mimeData,
                                     const QStringList& formats,
                                     const QString& text,
                                     QWidget* widget,
                                     bool clipboard )
{
    QStringList formatLabels;
    for ( int i = 0; i < formats.size(); ++i ) {
        const QString& fmt = formats[i];
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
    const QString chosenFormat = formats[ dlg.comboItem() ];

    kdDebug() << " result=" << result << " chosenFormat=" << chosenFormat << endl;
    KURL newUrl( u );
    newUrl.addPath( result );
    // if "data" came from QClipboard, then it was deleted already - by a nice 0-seconds timer
    // In that case, get it again. Let's hope the user didn't copy something else meanwhile :/
    // #### QT4/KDE4 TODO: check that this is still the case
    if ( clipboard ) {
        mimeData = QApplication::clipboard()->mimeData();
    }
    const QByteArray ba = mimeData->data( chosenFormat );
    return pasteDataAsyncTo( newUrl, ba );
}
#endif


#ifndef QT_NO_MIMECLIPBOARD
// The main method for dropping
KIO::CopyJob* KIO::pasteMimeSource( const QMimeData* mimeData, const KURL& destUrl,
                                    const QString& dialogText, QWidget* widget, bool clipboard )
{
  QByteArray ba;

  // Now check for plain text
  // We don't want to display a mimetype choice for a QTextDrag, those mimetypes look ugly.
  QString text;
  if ( mimeData->hasText() )
  {
      ba = mimeData->text().toLocal8Bit(); // encoding OK?
  }
  else
  {
      QStringList formats;
      const QStringList allFormats = mimeData->formats();
      for ( QStringList::const_iterator it = formats.begin(), end = formats.end() ;
            it != end ; ++it ) {
          if ( (*it) == QLatin1String( "application/x-qiconlist" ) ) // see QIconDrag
              continue;
          if ( (*it) == QLatin1String( "application/x-kde-cutselection" ) ) // see KonqDrag
              continue;
           if ( !(*it).contains( QLatin1Char( '/' ) ) ) // e.g. TARGETS, MULTIPLE, TIMESTAMP
              continue;
          formats.append( (*it) );
      }

      if ( formats.size() > 1 ) {
          return chooseAndPaste( destUrl, mimeData, formats, dialogText, widget, clipboard );
      }
      ba = mimeData->data( formats.first() );
  }
  if ( ba.isEmpty() )
  {
    KMessageBox::sorry( widget, i18n("The clipboard is empty") );
    return 0;
  }

  return pasteDataAsync( destUrl, ba, dialogText );
}
#endif

// The main method for pasting
KIO_EXPORT KIO::Job *KIO::pasteClipboard( const KURL& destUrl, QWidget* widget, bool move )
{
  if ( !destUrl.isValid() ) {
    KMessageBox::error( widget, i18n( "Malformed URL\n%1" ).arg( destUrl.url() ) );
    return 0;
  }

#ifndef QT_NO_MIMECLIPBOARD
  const QMimeData *mimeData = QApplication::clipboard()->mimeData();

  // First check for URLs.
  const KURL::List urls = KURL::List::fromMimeData( mimeData );
  if ( !urls.isEmpty() ) {
    KIO::Job *res = 0;
    if ( move )
      res = KIO::move( urls, destUrl );
    else
      res = KIO::copy( urls, destUrl );

    // If moving, erase the clipboard contents, the original files don't exist anymore
    if ( move )
      QApplication::clipboard()->clear();
    return res;
  }
  return pasteMimeSource( mimeData, destUrl, QString::null, widget, true /*clipboard*/ );
#else
  QByteArray ba;
  QTextStream txtStream( ba, QIODevice::WriteOnly );
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
  return pasteDataAsync( destUrl, ba );
#endif
}


KIO_EXPORT void KIO::pasteData( const KURL& u, const QByteArray& _data, QWidget* widget )
{
    const KURL newUrl = getNewFileName( u, QString::null );
    // We could use KIO::put here, but that would require a class
    // for the slotData call. With NetAccess, we can do a synchronous call.

    if (newUrl.isEmpty())
       return;

    KTempFile tempFile;
    tempFile.setAutoDelete( true );
    tempFile.dataStream()->writeRawData( _data.data(), _data.size() );
    tempFile.close();

    (void) KIO::NetAccess::upload( tempFile.name(), newUrl, widget );
}

KIO_EXPORT KIO::CopyJob* KIO::pasteDataAsync( const KURL& u, const QByteArray& _data, const QString& text )
{
    KURL newUrl = getNewFileName( u, text );

    if (newUrl.isEmpty())
       return 0;

    return pasteDataAsyncTo( newUrl, _data );
}

KIO_EXPORT QString KIO::pasteActionText()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    KURL::List urls = KURL::List::fromMimeData( mimeData );
    if ( !urls.isEmpty() ) {
        if ( urls.first().isLocalFile() )
            return i18n( "&Paste File", "&Paste %n Files", urls.count() );
        else
            return i18n( "&Paste URL", "&Paste %n URLs", urls.count() );
    } else if ( !mimeData->formats().isEmpty() ) {
        return i18n( "&Paste Clipboard Contents" );
    } else {
        return QString::null;
    }
}


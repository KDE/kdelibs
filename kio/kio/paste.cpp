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

#include "kio/copyjob.h"
#include "kio/deletejob.h"
#include "kio/global.h"
#include "kio/netaccess.h"
#include "kio/renamedialog.h"
#include "kio/kprotocolmanager.h"
#include "jobuidelegate.h"

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <ktemporaryfile.h>

#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QMimeData>
#include <QtCore/QTextIStream>

static KUrl getNewFileName( const KUrl &u, const QString& text, QWidget *widget )
{
  bool ok;
  QString dialogText( text );
  if ( dialogText.isEmpty() )
    dialogText = i18n( "Filename for clipboard content:" );
  QString file = KInputDialog::getText( QString(), dialogText, QString(), &ok, widget );
  if ( !ok )
     return KUrl();

  KUrl myurl(u);
  myurl.addPath( file );

  // Check for existing destination file. If we let CopyJob do it then we expose
  // an ugly tempfile name as the source URL...
  if (KIO::NetAccess::exists(myurl, KIO::NetAccess::DestinationSide, widget))
  {
      kDebug(7007) << "Paste will overwrite file.  Prompting...";
      KIO::RenameDialog_Result res = KIO::R_OVERWRITE;

      KIO::RenameDialog dlg( widget,
                          i18n("File Already Exists"),
                          u.pathOrUrl(),
                          myurl.pathOrUrl(),
                          (KIO::RenameDialog_Mode) (KIO::M_OVERWRITE | KIO::M_SINGLE) );
      res = static_cast<KIO::RenameDialog_Result>(dlg.exec());

      if ( res == KIO::R_RENAME )
      {
          myurl = dlg.newDestUrl();
      }
      else if ( res == KIO::R_CANCEL )
      {
          return KUrl();
      } else if (res == KIO::R_OVERWRITE)
      {
          // Ideally we would just pass KIO::Overwrite to the job in pasteDataAsyncTo.
          // But 1) CopyJob doesn't support that (it wouldn't really apply to multiple files)
          // 2) we can't use file_move because CopyJob* is everywhere in the API (see TODO)
          // As solution 1bis) we could use a PredefinedAnswerJobUiDelegate like in kiotesthelper.h...
          // But well the simpler is really to delete the dest:
          KIO::Job* delJob = KIO::del(myurl);
          delJob->exec();
      }
  }

  return myurl;
}

// The final step: write _data to tempfile and move it to newUrl
static KIO::CopyJob* pasteDataAsyncTo( const KUrl& newUrl, const QByteArray& _data )
{
    // ### Bug: because we move from a tempfile to the destination,
    // if the user does "Undo" then we won't ask for confirmation, and we'll
    // move back to a tempfile, instead of just deleting.
    // I guess we need a macro job which does copy+del?
    // A KIO::storedPut would be better but FileUndoManager would need to support it first.
    KTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    tempFile.open();
    tempFile.write(_data.data(), _data.size());
    tempFile.flush();

    KUrl origUrl(tempFile.fileName());

    return KIO::move(origUrl, newUrl);
}

#ifndef QT_NO_MIMECLIPBOARD
static KIO::CopyJob* chooseAndPaste( const KUrl& u, const QMimeData* mimeData,
                                     const QStringList& formats,
                                     const QString& text,
                                     QWidget* widget,
                                     bool clipboard )
{
    QStringList formatLabels;
    for ( int i = 0; i < formats.size(); ++i ) {
        const QString& fmt = formats[i];
        KMimeType::Ptr mime = KMimeType::mimeType(fmt, KMimeType::ResolveAliases);
        if (mime)
            formatLabels.append( i18n("%1 (%2)", mime->comment(), fmt) );
        else
            formatLabels.append( fmt );
    }

    QString dialogText( text );
    if ( dialogText.isEmpty() )
        dialogText = i18n( "Filename for clipboard content:" );
    //using QString() instead of QString::null didn't compile (with gcc 3.2.3), because the ctor was mistaken as a function declaration, Alex //krazy:exclude=nullstrassign
    KIO::PasteDialog dlg( QString::null, dialogText, QString(), formatLabels, widget, clipboard ); //krazy:exclude=nullstrassign

    if ( dlg.exec() != KDialog::Accepted )
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

    kDebug() << " result=" << result << " chosenFormat=" << chosenFormat;
    KUrl newUrl( u );
    newUrl.addPath( result );
    // if "data" came from QClipboard, then it was deleted already - by a nice 0-seconds timer
    // In that case, get it again. Let's hope the user didn't copy something else meanwhile :/
    // #### QT4/KDE4 TODO: check that this is still the case
    if ( clipboard ) {
        mimeData = QApplication::clipboard()->mimeData();
    }
    const QByteArray ba = mimeData->data( chosenFormat );
    KIO::CopyJob* job = pasteDataAsyncTo( newUrl, ba );
    job->ui()->setWindow(widget);
    return job;
}
#endif


#ifndef QT_NO_MIMECLIPBOARD

static QStringList extractFormats(const QMimeData* mimeData)
{
    QStringList formats;
    const QStringList allFormats = mimeData->formats();
    for (QStringList::const_iterator it = allFormats.constBegin(), end = allFormats.constEnd();
         it != end; ++it) {
        if ((*it) == QLatin1String("application/x-qiconlist")) // see QIconDrag
            continue;
        if ((*it) == QLatin1String("application/x-kde-cutselection")) // see KonqDrag
            continue;
        if (!(*it).contains(QLatin1Char('/'))) // e.g. TARGETS, MULTIPLE, TIMESTAMP
            continue;
        formats.append(*it);
    }
    return formats;
}

// The main method for dropping
KIO::CopyJob* KIO::pasteMimeSource( const QMimeData* mimeData, const KUrl& destUrl,
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
      const QStringList formats = extractFormats(mimeData);
      if ( formats.size() == 0 )
          return 0;

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

  return pasteDataAsync( destUrl, ba, widget, dialogText );
}

KIO_EXPORT bool KIO::canPasteMimeSource(const QMimeData* data)
{
    return data->hasText() || !extractFormats(data).isEmpty();
}

#endif

// The main method for pasting
KIO_EXPORT KIO::Job *KIO::pasteClipboard( const KUrl& destUrl, QWidget* widget, bool move )
{
  if ( !destUrl.isValid() ) {
    KMessageBox::error( widget, i18n( "Malformed URL\n%1", destUrl.prettyUrl() ) );
    return 0;
  }

#ifndef QT_NO_MIMECLIPBOARD
  const QMimeData *mimeData = QApplication::clipboard()->mimeData();

  // First check for URLs.
  const KUrl::List urls = KUrl::List::fromMimeData(mimeData, KUrl::List::PreferLocalUrls);
  if ( !urls.isEmpty() ) {
    KIO::Job *res = 0;   
    if ( move )
      res = KIO::move( urls, destUrl );
    else
      res = KIO::copy( urls, destUrl );
    res->ui()->setWindow(widget);

    // If moving, update the clipboard contents with the new locations
    if ( move ) {
      QApplication::clipboard()->clear();

      KUrl::List newUrls;
      for (int i = 0; i < urls.size(); i++) {
          KUrl dUrl = destUrl;
          dUrl.addPath(urls.at(i).fileName());
          newUrls.append(dUrl);
      }

      QMimeData* mime = new QMimeData();
      newUrls.populateMimeData(mime);
      QApplication::clipboard()->setMimeData(mime);
    }  
      
    return res;
  }
  return pasteMimeSource( mimeData, destUrl, QString(), widget, true /*clipboard*/ );
#else
  QByteArray ba;
  QTextStream txtStream( ba, QIODevice::WriteOnly );

  const QStringList data = QApplication::clipboard()->text().split('\n', QString::SkipEmptyParts);

  KUrl::List urls;
  KURLDrag::decode(data, urls);
  QStringList::const_iterator end(data.end());
  for(QStringList::const_iterator it=data.begin(); it!=end; ++it)
      txtStream << *it;
  if ( ba.size() == 0 )
  {
    KMessageBox::sorry(widget, i18n("The clipboard is empty"));
    return 0;
  }
  return pasteDataAsync( destUrl, ba, widget );
#endif
}


KIO_EXPORT void KIO::pasteData( const KUrl& u, const QByteArray& _data, QWidget* widget )
{
    const KUrl newUrl = getNewFileName( u, QString(), widget );
    // We could use KIO::put here, but that would require a class
    // for the slotData call. With NetAccess, we can do a synchronous call.

    if (newUrl.isEmpty())
       return;

    KTemporaryFile tempFile;
    tempFile.open();
    tempFile.write( _data.data(), _data.size() );
    tempFile.flush();

    (void) KIO::NetAccess::upload( tempFile.fileName(), newUrl, widget );
}

// KDE5 TODO: return a KIO::Job*, not a CopyJob*, in case we want to use file_move or a macro job...
// But then the caller needs to know the destUrl too. Return a QPair?
KIO_EXPORT KIO::CopyJob* KIO::pasteDataAsync( const KUrl& u, const QByteArray& _data, QWidget *widget, const QString& text )
{
    KUrl newUrl = getNewFileName( u, text, widget );

    if (newUrl.isEmpty())
       return 0;

    KIO::CopyJob* job = pasteDataAsyncTo( newUrl, _data );
    job->ui()->setWindow(widget);
    return job;
}

// NOTE: DolphinView::pasteInfo() has a better version of this
// (but which requires KonqFileItemCapabilities)
KIO_EXPORT QString KIO::pasteActionText()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    const KUrl::List urls = KUrl::List::fromMimeData( mimeData );
    if ( !urls.isEmpty() ) {
        if ( urls.first().isLocalFile() )
            return i18np( "&Paste File", "&Paste %1 Files", urls.count() );
        else
            return i18np( "&Paste URL", "&Paste %1 URLs", urls.count() );
    } else if ( !mimeData->formats().isEmpty() ) {
        return i18n( "&Paste Clipboard Contents" );
    } else {
        return QString();
    }
}

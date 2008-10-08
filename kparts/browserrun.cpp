/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <faure@kde.org>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "browserrun.h"
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/scheduler.h>
#include <kio/copyjob.h>
#include <klocale.h>
#include <kshell.h>
#include <kstringhandler.h>
#include <kmimetypetrader.h>
#include <ktemporaryfile.h>
#include <kdebug.h>
#include <kde_file.h>
#include <kstandarddirs.h>
#include <assert.h>

using namespace KParts;

class BrowserRun::BrowserRunPrivate
{
public:
    bool m_bHideErrorDialog;
    bool m_bRemoveReferrer;
    bool m_bTrustedSource;
    KParts::OpenUrlArguments m_args;
    KParts::BrowserArguments m_browserArgs;

    KParts::ReadOnlyPart *m_part; // QGuardedPtr?
    QPointer<QWidget> m_window;
    QString m_mimeType;
    QString m_contentDisposition;
};

BrowserRun::BrowserRun( const KUrl& url, const KParts::OpenUrlArguments& args,
                        const KParts::BrowserArguments& browserArgs,
                        KParts::ReadOnlyPart *part, QWidget* window,
                        bool removeReferrer, bool trustedSource, bool hideErrorDialog )
    : KRun( url, window, 0 /*mode*/, false /*is_local_file known*/, false /* no GUI */ ),
      d(new BrowserRunPrivate)
{
    d->m_bHideErrorDialog = hideErrorDialog;
    d->m_bRemoveReferrer = removeReferrer;
    d->m_bTrustedSource = trustedSource;
    d->m_args = args;
    d->m_browserArgs = browserArgs;
    d->m_part = part;
    d->m_window = window;
}

BrowserRun::~BrowserRun()
{
  delete d;
}

KParts::ReadOnlyPart* BrowserRun::part() const
{
    return d->m_part;
}

KUrl BrowserRun::url() const
{
    return KRun::url();
}

void BrowserRun::init()
{
  if ( d->m_bHideErrorDialog )
  {
    // ### KRun doesn't call a virtual method when it finds out that the URL
    // is either malformed, or points to a non-existing local file...
    // So we need to reimplement some of the checks, to handle d->m_bHideErrorDialog
    if ( !KRun::url().isValid() ) {
        redirectToError( KIO::ERR_MALFORMED_URL, KRun::url().url() );
        return;
    }
    if ( !isLocalFile() && !hasError() && KRun::url().isLocalFile() )
      setIsLocalFile( true );

    if ( isLocalFile() )  {
      KDE_struct_stat buff;
      if ( KDE_stat( QFile::encodeName(KRun::url().toLocalFile()), &buff ) == -1 )
      {
        kDebug(1000) << KRun::url().toLocalFile() << "doesn't exist.";
        redirectToError( KIO::ERR_DOES_NOT_EXIST, KRun::url().toLocalFile() );
        return;
      }
      setMode( buff.st_mode ); // while we're at it, save it for KRun::init() to use it
    }
  }
  KRun::init();
}

void BrowserRun::scanFile()
{
  kDebug(1000) << KRun::url();

  // Let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  // Optimization for http/https, findByURL doesn't trust extensions over http.
  if ( KRun::url().query().isEmpty() && !KRun::url().protocol().startsWith("http") )
  {
    KMimeType::Ptr mime = KMimeType::findByUrl( KRun::url() );
    assert( mime );
    if ( mime->name() != "application/octet-stream" || isLocalFile() )
    {
      kDebug(1000) << "MIME TYPE is" << mime->name();
      mimeTypeDetermined( mime->name() );
      return;
    }
  }

    QMap<QString, QString>& metaData = d->m_args.metaData();
    if ( d->m_part ) {
        const QString proto = d->m_part->url().protocol().toLower();

      if (proto == "https" || proto == "webdavs") {
            metaData.insert("main_frame_request", "TRUE" );
            metaData.insert("ssl_was_in_use", "TRUE" );
            metaData.insert("ssl_activate_warnings", "TRUE" );
      } else if (proto == "http" || proto == "webdav") {
            metaData.insert("ssl_activate_warnings", "TRUE" );
            metaData.insert("ssl_was_in_use", "FALSE" );
      }

      // Set the PropagateHttpHeader meta-data if it has not already been set...
        if (!metaData.contains("PropagateHttpHeader"))
            metaData.insert("PropagateHttpHeader", "TRUE");
  }

  KIO::TransferJob *job;
    if ( d->m_browserArgs.doPost() && KRun::url().protocol().startsWith("http")) {
        job = KIO::http_post( KRun::url(), d->m_browserArgs.postData, KIO::HideProgressInfo );
        job->addMetaData( "content-type", d->m_browserArgs.contentType() );
    } else {
        job = KIO::get(KRun::url(),
                       d->m_args.reload() ? KIO::Reload : KIO::NoReload,
                       KIO::HideProgressInfo);
  }

    if ( d->m_bRemoveReferrer )
        metaData.remove("referrer");

    job->addMetaData( metaData );
    job->ui()->setWindow( d->m_window );
  connect( job, SIGNAL( result( KJob *)),
           this, SLOT( slotBrowserScanFinished(KJob *)));
  connect( job, SIGNAL( mimetype( KIO::Job *, const QString &)),
           this, SLOT( slotBrowserMimetype(KIO::Job *, const QString &)));
  setJob( job );
}

void BrowserRun::slotBrowserScanFinished(KJob *job)
{
  kDebug(1000) << job->error();
  if ( job->error() == KIO::ERR_IS_DIRECTORY )
  {
      // It is in fact a directory. This happens when HTTP redirects to FTP.
      // Due to the "protocol doesn't support listing" code in BrowserRun, we
      // assumed it was a file.
      kDebug(1000) << "It is in fact a directory!";
      // Update our URL in case of a redirection
      KRun::setUrl( static_cast<KIO::TransferJob *>(job)->url() );
      setJob( 0 );
      mimeTypeDetermined( "inode/directory" );
  }
  else
  {
      if ( job->error() )
          handleError( job );
      else
          KRun::slotScanFinished(job);
  }
}

void BrowserRun::slotBrowserMimetype( KIO::Job *_job, const QString &type )
{
    Q_ASSERT( _job == KRun::job() );
    KIO::TransferJob *job = static_cast<KIO::TransferJob *>(KRun::job());
    // Update our URL in case of a redirection
    //kDebug(1000) << "old URL=" << KRun::url();
    //kDebug(1000) << "new URL=" << job->url();
    setUrl( job->url() );

    if (job->isErrorPage()) {
        d->m_mimeType = type;
        handleError(job);
        setJob( 0 );
    } else {
        kDebug(1000) << "found" << type << "for" << KRun::url();

        // Suggested filename given by the server (e.g. HTTP content-disposition)
        // When set, we should really be saving instead of embedding
        const QString suggestedFileName = job->queryMetaData("content-disposition-filename");
        setSuggestedFileName(suggestedFileName); // store it (in KRun)
        //kDebug(1000) << "suggestedFileName=" << suggestedFileName;
        d->m_contentDisposition = job->queryMetaData("content-disposition-type");

        // Make a copy to avoid a dead reference
        QString _type = type;
        job->putOnHold();
        setJob( 0 );

        mimeTypeDetermined( _type );
    }
}

BrowserRun::NonEmbeddableResult BrowserRun::handleNonEmbeddable( const QString& _mimeType )
{
    QString mimeType( _mimeType );
    Q_ASSERT( !hasFinished() ); // only come here if the mimetype couldn't be embedded
    // Support for saving remote files.
    if ( mimeType != "inode/directory" && // dirs can't be saved
         !KRun::url().isLocalFile() )
    {
        if ( isTextExecutable(mimeType) )
            mimeType = QLatin1String("text/plain"); // view, don't execute
        kDebug(1000) << "ask for saving";
        KService::Ptr offer = KMimeTypeTrader::self()->preferredService(mimeType, "Application");
        // ... -> ask whether to save
        KParts::BrowserRun::AskSaveResult res = askSave( KRun::url(), offer, mimeType, suggestedFileName() );
        if ( res == KParts::BrowserRun::Save ) {
            save( KRun::url(), suggestedFileName() );
            kDebug(1000) << "Save: returning Handled";
            setFinished( true );
            return Handled;
        }
        else if ( res == KParts::BrowserRun::Cancel ) {
            // saving done or canceled
            kDebug(1000) << "Cancel: returning Handled";
            setFinished( true );
            return Handled;
        }
        else // "Open" chosen (done by KRun::foundMimeType, called when returning NotHandled)
        {
            // If we were in a POST, we can't just pass a URL to an external application.
            // We must save the data to a tempfile first.
            if ( d->m_browserArgs.doPost() )
            {
                kDebug(1000) << "request comes from a POST, can't pass a URL to another app, need to save";
                d->m_mimeType = mimeType;
                QString extension;
                QString fileName = suggestedFileName().isEmpty() ? KRun::url().fileName() : suggestedFileName();
                int extensionPos = fileName.lastIndexOf( '.' );
                if ( extensionPos != -1 )
                    extension = fileName.mid( extensionPos ); // keep the '.'
                KTemporaryFile tempFile;
                tempFile.setSuffix(extension);
                tempFile.setAutoRemove(false);
                tempFile.open();
                KUrl destURL;
                destURL.setPath( tempFile.fileName() );
                KIO::Job *job = KIO::file_copy( KRun::url(), destURL, 0600, KIO::Overwrite );
                job->ui()->setWindow(d->m_window);
                connect( job, SIGNAL(result(KJob *)),
                         this, SLOT(slotCopyToTempFileResult(KJob *)) );
                return Delayed; // We'll continue after the job has finished
            }
        }
    }

    // Check if running is allowed
    if ( !d->m_bTrustedSource && // ... and untrusted source...
         !allowExecution( mimeType, KRun::url() ) ) // ...and the user said no (for executables etc.)
    {
        setFinished( true );
        return Handled;
    }

    KIO::SimpleJob::removeOnHold(); // Kill any slave that was put on hold.
    return NotHandled;
}

//static
bool BrowserRun::allowExecution( const QString &mimeType, const KUrl &url )
{
    if ( !KRun::isExecutable( mimeType ) )
      return true;

    if ( !url.isLocalFile() ) // Don't permit to execute remote files
        return false;

    return ( KMessageBox::warningContinueCancel( 0,
                                                 i18n( "Do you really want to execute '%1'?", url.prettyUrl() ),
    i18n("Execute File?"), KGuiItem(i18n("Execute")) ) == KMessageBox::Continue );
}

static QString makeQuestion( const KUrl& url, const QString& mimeType, const QString& suggestedFileName )
{
    QString surl = url.prettyUrl();
    KMimeType::Ptr mime = KMimeType::mimeType(mimeType, KMimeType::ResolveAliases);
    QString comment = mimeType;

    // Test if the mimeType is not recognize as octet-stream.
    // If so then keep mime-type as comment
    if (mime && mime->name() != KMimeType::defaultMimeType()) {
        // The mime-type is known so display the comment instead of mime-type
        comment = mime->comment();
    }
    // The strange order in the i18n() calls below is due to the possibility
    // of surl containing a '%'
    if ( suggestedFileName.isEmpty() )
        return i18n("Open '%2'?\nType: %1", comment, surl);
    else
        return i18n("Open '%3'?\nName: %2\nType: %1", comment, suggestedFileName, surl);
}

//static
// TODO should take a QWidget* parent argument
BrowserRun::AskSaveResult BrowserRun::askSave( const KUrl & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFileName )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with kdebase/kcontrol/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    QString question = makeQuestion( url, mimeType, suggestedFileName );

    // Text used for the open button
    QString openText = (offer && !offer->name().isEmpty())
                       ? i18n("&Open with '%1'", offer->name())
                       : i18n("&Open With...");

    int choice = KMessageBox::questionYesNoCancel(
        0, question, url.host(),
        KStandardGuiItem::saveAs(), KGuiItem(openText), KStandardGuiItem::cancel(),
        QLatin1String("askSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!

    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

//static
// TODO should take a QWidget* parent argument
BrowserRun::AskSaveResult BrowserRun::askEmbedOrSave( const KUrl & url, const QString& mimeType, const QString & suggestedFileName, int flags )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with
    // kdebase/apps/konqueror/settings/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    KMimeType::Ptr mime = KMimeType::mimeType(mimeType, KMimeType::ResolveAliases);
    // Don't ask for:
    // - html (even new tabs would ask, due to about:blank!)
    // - dirs obviously (though not common over HTTP :),
    // - images (reasoning: no need to save, most of the time, because fast to see)
    // e.g. postscript is different, because takes longer to read, so
    // it's more likely that the user might want to save it.
    // - multipart/* ("server push", see kmultipart)
    // - other strange 'internal' mimetypes like print/manager...
    // KEEP IN SYNC!!!
    if (flags != (int)AttachmentDisposition && mime && (
         mime->is( "text/html" ) ||
         mime->is( "application/xml" ) ||
         mime->is( "inode/directory" ) ||
         mimeType.startsWith( "image" ) ||
         mime->is( "multipart/x-mixed-replace" ) ||
         mime->is( "multipart/replace" ) ||
         mimeType.startsWith( "print" ) ) )
        return Open;

    QString question = makeQuestion( url, mimeType, suggestedFileName );

    // don't use KStandardGuiItem::open() here which has trailing ellipsis!
    int choice = KMessageBox::questionYesNoCancel(
        0, question, url.host(),
        KStandardGuiItem::saveAs(), KGuiItem( i18n( "&Open" ), "document-open"), KStandardGuiItem::cancel(),
        QLatin1String("askEmbedOrSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!
    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

// Default implementation, overridden in KHTMLRun
void BrowserRun::save( const KUrl & url, const QString & suggestedFileName )
{
    simpleSave( url, suggestedFileName, d->m_window );
}

// static
void BrowserRun::simpleSave( const KUrl & url, const QString & suggestedFileName,
                             QWidget* window )
{
    // DownloadManager <-> konqueror integration
    // find if the integration is enabled
    // the empty key  means no integration
    // only use the downloadmanager for non-local urls
    if ( !url.isLocalFile() )
    {
        KConfigGroup cfg = KSharedConfig::openConfig("konquerorrc", KConfig::NoGlobals)->group("HTML Settings");
        QString downloadManger = cfg.readPathEntry("DownloadManager", QString());
        if (!downloadManger.isEmpty())
        {
            // then find the download manager location
            kDebug(1000) << "Using: "<<downloadManger <<" as Download Manager";
            QString cmd=KStandardDirs::findExe(downloadManger);
            if (cmd.isEmpty())
            {
                QString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ", downloadManger);
                QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled.");
                KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                cfg.writePathEntry("DownloadManager",QString());
                cfg.sync ();
            }
            else
            {
                // ### suggestedFileName not taken into account. Fix this (and
                // the duplicated code) with shiny new KDownload class for 3.2 (pfeiffer)
                // Until the shiny new class comes about, send the suggestedFileName
                // along with the actual URL to download. (DA)
                cmd += " " + KShell::quoteArg(url.url());
                if ( !suggestedFileName.isEmpty() )
                    cmd +=" " + KShell::quoteArg(suggestedFileName);

                kDebug(1000) << "Calling command" << cmd;
                // slave is already on hold (slotBrowserMimetype())
                KIO::Scheduler::publishSlaveOnHold();
                KRun::runCommand(cmd, window);
                return;
            }
        }
    }

    // no download manager available, let's do it ourself
    KFileDialog *dlg = new KFileDialog( QString(), QString() /*all files*/,
                                        window);
    dlg->setOperationMode( KFileDialog::Saving );
    dlg->setCaption(i18n("Save As"));

    dlg->setSelection( suggestedFileName.isEmpty() ? url.fileName() : suggestedFileName );
    if ( dlg->exec() )
    {
        KUrl destURL( dlg->selectedUrl() );
        if ( destURL.isValid() )
        {
            KIO::Job *job = KIO::copy( url, destURL );
            job->ui()->setWindow (window);
            job->ui()->setAutoErrorHandlingEnabled( true );
        }
    }
    delete dlg;
}

void BrowserRun::slotStatResult( KJob *job )
{
    if ( job->error() ) {
        kDebug(1000) << job->errorString();
        handleError( job );
    } else
        KRun::slotStatResult( job );
}

void BrowserRun::handleError( KJob * job )
{
    if ( !job ) { // Shouldn't happen, see docu.
        kWarning(1000) << "handleError called with job=0! hideErrorDialog=" << d->m_bHideErrorDialog;
        return;
    }

    KIO::TransferJob *tjob = qobject_cast<KIO::TransferJob *>(job);
    if (tjob && tjob->isErrorPage() && !job->error()) {
        // The default handling of error pages is to show them like normal pages
        // But this is done here in handleError so that KHTMLRun can reimplement it
        tjob->putOnHold();
        setJob(0);
        if (!d->m_mimeType.isEmpty())
            mimeTypeDetermined(d->m_mimeType);
        return;
    }

    if (d->m_bHideErrorDialog && job->error() != KIO::ERR_NO_CONTENT)
    {
        redirectToError( job->error(), job->errorText() );
        return;
    }

    // Reuse code in KRun, to benefit from d->m_showingError etc.
    KRun::slotStatResult( job );
}

void BrowserRun::redirectToError( int error, const QString& errorText )
{
    /**
     * To display this error in KHTMLPart instead of inside a dialog box,
     * we tell konq that the mimetype is text/html, and we redirect to
     * an error:/ URL that sends the info to khtml.
     *
     * The format of the error:/ URL is error:/?query#url,
     * where two variables are passed in the query:
     * error = int kio error code, errText = QString error text from kio
     * The sub-url is the URL that we were trying to open.
     */
    KUrl newURL(QString("error:/?error=%1&errText=%2")
                .arg( error )
                .arg( QString::fromUtf8( QUrl::toPercentEncoding( errorText ) ) ) );
    KUrl runURL = KRun::url();
    runURL.setPass( QString() ); // don't put the password in the error URL

    KUrl::List lst;
    lst << newURL << runURL;
    KRun::setUrl( KUrl::join( lst ) );

    setJob( 0 );
    mimeTypeDetermined( "text/html" );
}

void BrowserRun::slotCopyToTempFileResult(KJob *job)
{
    if ( job->error() ) {
	    job->uiDelegate()->showErrorMessage();
    } else {
        // Same as KRun::foundMimeType but with a different URL
        (void) (KRun::runUrl( static_cast<KIO::FileCopyJob *>(job)->destUrl(), d->m_mimeType, d->m_window ));
    }
    setError( true ); // see above
    setFinished( true );
    timer().start( 0 );
}

bool BrowserRun::isTextExecutable( const QString &mimeType )
{
    return ( mimeType == "application/x-desktop" ||
             mimeType == "application/x-shellscript" );
}

bool BrowserRun::hideErrorDialog() const
{
    return d->m_bHideErrorDialog;
}

QString BrowserRun::contentDisposition() const {
    return d->m_contentDisposition;
}

KParts::OpenUrlArguments& KParts::BrowserRun::arguments()
{
    return d->m_args;
}

KParts::BrowserArguments& KParts::BrowserRun::browserArguments()
{
    return d->m_browserArgs;
}

#include "browserrun.moc"

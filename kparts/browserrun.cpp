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
#include <kio/scheduler.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <assert.h>

using namespace KParts;

class BrowserRun::BrowserRunPrivate
{
public:
  bool m_bHideErrorDialog;
};

BrowserRun::BrowserRun( const KUrl& url, const KParts::URLArgs& args,
                        KParts::ReadOnlyPart *part, QWidget* window,
                        bool removeReferrer, bool trustedSource, bool hideErrorDialog )
    : KRun( url, window, 0 /*mode*/, false /*is_local_file known*/, false /* no GUI */ ),
      m_args( args ), m_part( part ), m_window( window ),
      m_bRemoveReferrer( removeReferrer ), m_bTrustedSource( trustedSource ),d(new BrowserRunPrivate)
{
  d->m_bHideErrorDialog = hideErrorDialog;
}

BrowserRun::~BrowserRun()
{
  delete d;
}

void BrowserRun::init()
{
  if ( d->m_bHideErrorDialog )
  {
    // ### KRun doesn't call a virtual method when it finds out that the URL
    // is either malformed, or points to a non-existing local file...
    // So we need to reimplement some of the checks, to handle m_bHideErrorDialog
    if ( !m_strURL.isValid() ) {
        redirectToError( KIO::ERR_MALFORMED_URL, m_strURL.url() );
        return;
    }
    if ( !m_bIsLocalFile && !m_bFault && m_strURL.isLocalFile() )
      m_bIsLocalFile = true;

    if ( m_bIsLocalFile )  {
      struct stat buff;
      if ( stat( QFile::encodeName(m_strURL.path()), &buff ) == -1 )
      {
        kDebug(1000) << "BrowserRun::init : " << m_strURL.prettyURL() << " doesn't exist." << endl;
        redirectToError( KIO::ERR_DOES_NOT_EXIST, m_strURL.path() );
        return;
      }
      m_mode = buff.st_mode; // while we're at it, save it for KRun::init() to use it
    }
  }
  KRun::init();
}

void BrowserRun::scanFile()
{
  kDebug(1000) << "BrowserRun::scanfile " << m_strURL.prettyURL() << endl;

  // Let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  // Optimization for http/https, findByURL doesn't trust extensions over http.
  if ( m_strURL.query().isEmpty() && !m_strURL.protocol().startsWith("http") )
  {
    KMimeType::Ptr mime = KMimeType::findByURL( m_strURL );
    assert( mime );
    if ( mime->name() != "application/octet-stream" || m_bIsLocalFile )
    {
      kDebug(1000) << "Scanfile: MIME TYPE is " << mime->name() << endl;
      foundMimeType( mime->name() );
      return;
    }
  }

  if ( m_part )
  {
      QString proto = m_part->url().protocol().toLower();

      if (proto == "https" || proto == "webdavs") {
          m_args.metaData().insert("main_frame_request", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "TRUE" );
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
      } else if (proto == "http" || proto == "webdav") {
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "FALSE" );
      }

      // Set the PropagateHttpHeader meta-data if it has not already been set...
      if (!m_args.metaData().contains("PropagateHttpHeader"))
          m_args.metaData().insert("PropagateHttpHeader", "TRUE");
  }

  KIO::TransferJob *job;
  if ( m_args.doPost() && m_strURL.protocol().startsWith("http"))
  {
      job = KIO::http_post( m_strURL, m_args.postData, false );
      job->addMetaData( "content-type", m_args.contentType() );
  }
  else
      job = KIO::get(m_strURL, m_args.reload, false);

  if ( m_bRemoveReferrer )
     m_args.metaData().remove("referrer");

  job->addMetaData( m_args.metaData() );
  job->setWindow( m_window );
  connect( job, SIGNAL( result( KIO::Job *)),
           this, SLOT( slotBrowserScanFinished(KIO::Job *)));
  connect( job, SIGNAL( mimetype( KIO::Job *, const QString &)),
           this, SLOT( slotBrowserMimetype(KIO::Job *, const QString &)));
  m_job = job;
}

void BrowserRun::slotBrowserScanFinished(KIO::Job *job)
{
  kDebug(1000) << "BrowserRun::slotBrowserScanFinished" << endl;
  if ( job->error() == KIO::ERR_IS_DIRECTORY )
  {
      // It is in fact a directory. This happens when HTTP redirects to FTP.
      // Due to the "protocol doesn't support listing" code in BrowserRun, we
      // assumed it was a file.
      kDebug(1000) << "It is in fact a directory!" << endl;
      // Update our URL in case of a redirection
      m_strURL = static_cast<KIO::TransferJob *>(job)->url();
      m_job = 0;
      foundMimeType( "inode/directory" );
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
  Q_ASSERT( _job == m_job );
  KIO::TransferJob *job = static_cast<KIO::TransferJob *>(m_job);
  // Update our URL in case of a redirection
  //kDebug(1000) << "old URL=" << m_strURL.url() << endl;
  //kDebug(1000) << "new URL=" << job->url().url() << endl;
  m_strURL = job->url();
  kDebug(1000) << "slotBrowserMimetype: found " << type << " for " << m_strURL.prettyURL() << endl;

  m_suggestedFilename = job->queryMetaData("content-disposition");
  //kDebug(1000) << "m_suggestedFilename=" << m_suggestedFilename << endl;

  // Make a copy to avoid a dead reference
  QString _type = type;
  job->putOnHold();
  m_job = 0;

  foundMimeType( _type );
}

BrowserRun::NonEmbeddableResult BrowserRun::handleNonEmbeddable( const QString& _mimeType )
{
    QString mimeType( _mimeType );
    Q_ASSERT( !m_bFinished ); // only come here if the mimetype couldn't be embedded
    // Support for saving remote files.
    if ( mimeType != "inode/directory" && // dirs can't be saved
         !m_strURL.isLocalFile() )
    {
        if ( isTextExecutable(mimeType) )
            mimeType = QLatin1String("text/plain"); // view, don't execute
        kDebug(1000) << "BrowserRun: ask for saving" << endl;
        KService::Ptr offer = KServiceTypeProfile::preferredService(mimeType, "Application");
        // ... -> ask whether to save
        KParts::BrowserRun::AskSaveResult res = askSave( m_strURL, offer, mimeType, m_suggestedFilename );
        if ( res == KParts::BrowserRun::Save ) {
            save( m_strURL, m_suggestedFilename );
            kDebug(1000) << "BrowserRun::handleNonEmbeddable: Save: returning Handled" << endl;
            m_bFinished = true;
            return Handled;
        }
        else if ( res == KParts::BrowserRun::Cancel ) {
            // saving done or canceled
            kDebug(1000) << "BrowserRun::handleNonEmbeddable: Cancel: returning Handled" << endl;
            m_bFinished = true;
            return Handled;
        }
        else // "Open" chosen (done by KRun::foundMimeType, called when returning NotHandled)
        {
            // If we were in a POST, we can't just pass a URL to an external application.
            // We must save the data to a tempfile first.
            if ( m_args.doPost() )
            {
                kDebug(1000) << "BrowserRun: request comes from a POST, can't pass a URL to another app, need to save" << endl;
                m_sMimeType = mimeType;
                QString extension;
                QString fileName = m_suggestedFilename.isEmpty() ? m_strURL.fileName() : m_suggestedFilename;
                int extensionPos = fileName.lastIndexOf( '.' );
                if ( extensionPos != -1 )
                    extension = fileName.mid( extensionPos ); // keep the '.'
                KTempFile tempFile( QString::null, extension );
                KUrl destURL;
                destURL.setPath( tempFile.name() );
                KIO::Job *job = KIO::file_copy( m_strURL, destURL, 0600, true /*overwrite*/, false /*no resume*/, true /*progress info*/ );
                job->setWindow (m_window);
                connect( job, SIGNAL( result( KIO::Job *)),
                         this, SLOT( slotCopyToTempFileResult(KIO::Job *)) );
                return Delayed; // We'll continue after the job has finished
            }
        }
    }

    // Check if running is allowed
    if ( !m_bTrustedSource && // ... and untrusted source...
         !allowExecution( mimeType, m_strURL ) ) // ...and the user said no (for executables etc.)
    {
        m_bFinished = true;
        return Handled;
    }

    KIO::SimpleJob::removeOnHold(); // Kill any slave that was put on hold.
    return NotHandled;
}

//static
bool BrowserRun::allowExecution( const QString &serviceType, const KUrl &url )
{
    if ( !KRun::isExecutable( serviceType ) )
      return true;

    if ( !url.isLocalFile() ) // Don't permit to execute remote files
        return false;

    return ( KMessageBox::warningContinueCancel( 0, i18n( "Do you really want to execute '%1'? " ).arg( url.prettyURL() ),
    i18n("Execute File?"), i18n("Execute") ) == KMessageBox::Continue );
}

static QString makeQuestion( const KUrl& url, const QString& mimeType, const QString& suggestedFilename )
{
    QString surl = KStringHandler::csqueeze( url.prettyURL() );
    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    QString comment = mimeType;

    // Test if the mimeType is not recognize as octet-stream.
    // If so then keep mime-type as comment
    if (mime->name() != KMimeType::defaultMimeType()) {
        // The mime-type is known so display the comment instead of mime-type
        comment = mime->comment();
    }
    // The strange order in the i18n() calls below is due to the possibility
    // of surl containing a '%'
    if ( suggestedFilename.isEmpty() )
        return i18n("Open '%2'?\nType: %1").arg(comment).arg(surl);
    else
        return i18n("Open '%3'?\nName: %2\nType: %1").arg(comment).arg(suggestedFilename).arg(surl);
}

//static
BrowserRun::AskSaveResult BrowserRun::askSave( const KUrl & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFilename )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with kdebase/kcontrol/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    QString question = makeQuestion( url, mimeType, suggestedFilename );

    // Text used for the open button
    QString openText = (offer && !offer->name().isEmpty())
                       ? i18n("&Open with '%1'").arg(offer->name())
                       : i18n("&Open With...");

    int choice = KMessageBox::questionYesNoCancel(
        0L, question, url.host(),
        KStdGuiItem::saveAs(), openText,
        QLatin1String("askSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!

    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

//static
BrowserRun::AskSaveResult BrowserRun::askEmbedOrSave( const KUrl & url, const QString& mimeType, const QString & suggestedFilename, int /*flags*/ )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this funcion in sync with kdebase/kcontrol/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    // Don't ask for:
    // - html (even new tabs would ask, due to about:blank!)
    // - dirs obviously (though not common over HTTP :),
    // - images (reasoning: no need to save, most of the time, because fast to see)
    // e.g. postscript is different, because takes longer to read, so
    // it's more likely that the user might want to save it.
    // - multipart/* ("server push", see kmultipart)
    // - other strange 'internal' mimetypes like print/manager...
    // KEEP IN SYNC!!!
    if ( mime->is( "text/html" ) ||
         mime->is( "text/xml" ) ||
         mime->is( "inode/directory" ) ||
         mimeType.startsWith( "image" ) ||
         mime->is( "multipart/x-mixed-replace" ) ||
         mime->is( "multipart/replace" ) ||
         mimeType.startsWith( "print" ) )
        return Open;

    QString question = makeQuestion( url, mimeType, suggestedFilename );

    int choice = KMessageBox::questionYesNoCancel(
        0L, question, url.host(),
        KStdGuiItem::saveAs(), KGuiItem( i18n( "&Open" ), "fileopen"),
        QLatin1String("askEmbedOrSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!
    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

// Default implementation, overridden in KHTMLRun
void BrowserRun::save( const KUrl & url, const QString & suggestedFilename )
{
    simpleSave( url, suggestedFilename, m_window );
}

void BrowserRun::simpleSave( const KUrl & url, const QString & suggestedFilename,
                             QWidget* window )
{
    // DownloadManager <-> konqueror integration
    // find if the integration is enabled
    // the empty key  means no integration
    // only use the downloadmanager for non-local urls
    if ( !url.isLocalFile() )
    {
        KConfig cfg("konquerorrc", false, false);
        cfg.setGroup("HTML Settings");
        QString downloadManger = cfg.readPathEntry("DownloadManager");
        if (!downloadManger.isEmpty())
        {
            // then find the download manager location
            kDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
            QString cmd=KStandardDirs::findExe(downloadManger);
            if (cmd.isEmpty())
            {
                QString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ").arg(downloadManger);
                QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled!");
                KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                cfg.writePathEntry("DownloadManager",QString());
                cfg.sync ();
            }
            else
            {
                // ### suggestedFilename not taken into account. Fix this (and
                // the duplicated code) with shiny new KDownload class for 3.2 (pfeiffer)
                // Until the shiny new class comes about, send the suggestedFilename
                // along with the actual URL to download. (DA)
                cmd += " " + KProcess::quote(url.url());
                if ( !suggestedFilename.isEmpty() )
                    cmd +=" " + KProcess::quote(suggestedFilename);

                kDebug(1000) << "Calling command  " << cmd << endl;
                // slave is already on hold (slotBrowserMimetype())
                KIO::Scheduler::publishSlaveOnHold();
                KRun::runCommand(cmd);
                return;
            }
        }
    }

    // no download manager available, let's do it ourself
    KFileDialog *dlg = new KFileDialog( QString(), QString() /*all files*/,
                                        window);
    dlg->setOperationMode( KFileDialog::Saving );
    dlg->setCaption(i18n("Save As"));

    dlg->setSelection( suggestedFilename.isEmpty() ? url.fileName() : suggestedFilename );
    if ( dlg->exec() )
    {
        KUrl destURL( dlg->selectedURL() );
        if ( destURL.isValid() )
        {
            KIO::Job *job = KIO::copy( url, destURL );
            job->setWindow (window);
            job->setAutoErrorHandlingEnabled( true );
        }
    }
    delete dlg;
}

void BrowserRun::slotStatResult( KIO::Job *job )
{
    if ( job->error() ) {
        kDebug(1000) << "BrowserRun::slotStatResult : " << job->errorString() << endl;
        handleError( job );
    } else
        KRun::slotStatResult( job );
}

void BrowserRun::handleError( KIO::Job * job )
{
    if ( !job ) { // Shouldn't happen, see docu.
        kWarning(1000) << "BrowserRun::handleError called with job=0! hideErrorDialog=" << d->m_bHideErrorDialog << endl;
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
    m_strURL.setPass( QString() ); // don't put the password in the error URL

    KUrl::List lst;
    lst << newURL << m_strURL;
    m_strURL = KUrl::join( lst );
    //kDebug(1202) << "BrowserRun::handleError m_strURL=" << m_strURL.prettyURL() << endl;

    m_job = 0;
    foundMimeType( "text/html" );
}

void BrowserRun::slotCopyToTempFileResult(KIO::Job *job)
{
    if ( job->error() ) {
        job->showErrorDialog( m_window );
    } else {
        // Same as KRun::foundMimeType but with a different URL
        (void) (KRun::runURL( static_cast<KIO::FileCopyJob *>(job)->destURL(), m_sMimeType ));
    }
    m_bFault = true; // see above
    m_bFinished = true;
    m_timer.start( 0 );
}

bool BrowserRun::isTextExecutable( const QString &serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "application/x-shellscript" );
}

bool BrowserRun::hideErrorDialog() const
{
    return d->m_bHideErrorDialog;
}

#include "browserrun.moc"

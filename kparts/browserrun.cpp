/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <david@mandrakesoft.com>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "browserrun.h"
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <assert.h>

using namespace KParts;

BrowserRun::BrowserRun( const KURL& url, const KParts::URLArgs& args,
                        KParts::ReadOnlyPart *part, QWidget* window,
                        bool removeReferrer, bool trustedSource )
    : KRun( url, 0 /*mode*/, false /*is_local_file known*/, false /* no GUI */ ),
      m_args( args ), m_part( part ), m_window( window ),
      m_bRemoveReferrer( removeReferrer ), m_bTrustedSource( trustedSource )
{
}

void BrowserRun::scanFile()
{
  kdDebug(1000) << "BrowserRun::scanfile " << m_strURL.prettyURL() << endl;

  // Let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  // Optimization for http/https, findByURL doesn't trust extensions over http.
  if ( m_strURL.query().isEmpty() && !m_strURL.protocol().startsWith("http") )
  {
    KMimeType::Ptr mime = KMimeType::findByURL( m_strURL );
    assert( mime != 0L );
    if ( mime->name() != "application/octet-stream" || m_bIsLocalFile )
    {
      kdDebug(1000) << "Scanfile: MIME TYPE is " << mime->name() << endl;
      foundMimeType( mime->name() );
      return;
    }
  }

  if ( m_part )
  {
      QString proto = m_part->url().protocol();
      if (proto.find("https", 0, false) == 0) {
          m_args.metaData().insert("main_frame_request", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "TRUE" );
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
      } else if (proto.find("http", 0, false) == 0) {
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "FALSE" );
      }
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
  kdDebug(1000) << "BrowserRun::slotBrowserScanFinished" << endl;
  if ( job->error() == KIO::ERR_IS_DIRECTORY )
  {
      // It is in fact a directory. This happens when HTTP redirects to FTP.
      // Due to the "protocol doesn't support listing" code in BrowserRun, we
      // assumed it was a file.
      kdDebug(1000) << "It is in fact a directory!" << endl;
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
  KIO::TransferJob *job = (KIO::TransferJob *) m_job;
  // Update our URL in case of a redirection
  //kdDebug(1000) << "old URL=" << m_strURL.url() << endl;
  //kdDebug(1000) << "new URL=" << job->url().url() << endl;
  m_strURL = job->url();
  kdDebug(1000) << "slotBrowserMimetype: found " << type << " for " << m_strURL.prettyURL() << endl;

  m_suggestedFilename = job->queryMetaData("content-disposition");
  //kdDebug(1000) << "m_suggestedFilename=" << m_suggestedFilename << endl;

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
            mimeType = QString::fromLatin1("text/plain"); // view, don't execute
        kdDebug(1000) << "BrowserRun: ask for saving" << endl;
        KService::Ptr offer = KServiceTypeProfile::preferredService(mimeType, "Application");
        // ... -> ask whether to save
        KParts::BrowserRun::AskSaveResult res = askSave( m_strURL, offer, mimeType, m_suggestedFilename );
        if ( res == KParts::BrowserRun::Save ) {
            save( m_strURL, m_suggestedFilename );
            m_bFinished = true;
            return Handled;
        }
        else if ( res == KParts::BrowserRun::Cancel ) {
            // saving done or canceled
            m_bFinished = true;
            return Handled;
        }
        else // "Open" chosen (done by KRun::foundMimeType, called when returning NotHandled)
        {
            // If we were in a POST, we can't just pass a URL to an external application.
            // We must save the data to a tempfile first.
            if ( m_args.doPost() )
            {
                kdDebug(1000) << "BrowserRun: request comes from a POST, can't pass a URL to another app, need to save" << endl;
                m_sMimeType = mimeType;
                QString extension;
                QString fileName = m_suggestedFilename.isEmpty() ? m_strURL.fileName() : m_suggestedFilename;
                int extensionPos = fileName.findRev( '.' );
                if ( extensionPos != -1 )
                    extension = fileName.mid( extensionPos ); // keep the '.'
                KTempFile tempFile( QString::null, extension );
                KURL destURL;
                destURL.setPath( tempFile.name() );
                KIO::Job *job = KIO::file_copy( m_strURL, destURL, 0600, true /*overwrite*/, false /*no resume*/, true /*progress info*/ );
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
bool BrowserRun::allowExecution( const QString &serviceType, const KURL &url )
{
    if ( !isExecutable( serviceType ) )
      return true;

    if ( !url.isLocalFile() ) // Don't permit to execute remote files
        return false;

    return ( KMessageBox::warningYesNo( 0, i18n( "Do you really want to execute '%1'? " ).arg( url.prettyURL() ) ) == KMessageBox::Yes );
}

//static
BrowserRun::AskSaveResult BrowserRun::askSave( const KURL & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFilename )
{
    QString surl = KStringHandler::csqueeze( url.prettyURL() );
    QString question;
    if ( suggestedFilename.isEmpty() )
    {
        question = offer ? i18n("Open '%1' using '%2'?").
                   arg( surl ).arg(offer->name())
                   : i18n("Open '%1'?").arg( surl );
    } else {
        question = offer ? i18n("Open '%1' (%2) using '%3'?").
                   arg( surl ).arg(suggestedFilename).arg(offer->name())
                   : i18n("Open '%1' (%2)?").arg( surl ).arg(suggestedFilename);
    }
    int choice = KMessageBox::questionYesNoCancel(
        0L, question, QString::null,
        i18n("&Save to Disk"), i18n("&Open"),
        QString::fromLatin1("askSave")+ mimeType ); // dontAskAgainName
    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
}
// Default implementation, overriden in KHTMLRun
void BrowserRun::save( const KURL & url, const QString & suggestedFilename )
{

    // DownloadManager <-> konqueror integration
    // find if the integration is enabled
    // the empty key  means no integration
    KConfig *cfg = new KConfig("konquerorrc", false, false);
    cfg->setGroup("HTML Settings");
    QString downloadManger=cfg->readEntry("DownloadManager");
    if (!downloadManger.isEmpty())
    {
        // then find the download manager location 
        kdDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
        QString cmd=KStandardDirs::findExe(downloadManger);
        if (cmd.isEmpty())
        {
            QString errMsg=i18n("Sorry, I can't find  the Download Manager (%1) in your $PATH ").arg(downloadManger);
            QString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with konqueror will be disabled!");
            KMessageBox::detailedSorry(0,errMsg,errMsgEx);
            cfg->writeEntry("DownloadManager",QString::null);
            cfg->sync ();
        }
        else
        {
	     cmd+=" " + url.url(); 
            kdDebug(1000) << "Calling command  "<<cmd<<endl;
            KRun::runCommand(cmd);
        }
    }
    else
        simpleSave( url, suggestedFilename );
    delete cfg;
}

// static
void BrowserRun::simpleSave( const KURL & url, const QString & suggestedFilename )
{
    KFileDialog *dlg = new KFileDialog( QString::null, QString::null /*all files*/,
                                        0L , "filedialog", true );

    dlg->setKeepLocation( true );

    dlg->setCaption(i18n("Save As"));

    dlg->setSelection( suggestedFilename.isEmpty() ? url.fileName() : suggestedFilename );
    if ( dlg->exec() )
    {
        KURL destURL( dlg->selectedURL() );
        if ( !destURL.isMalformed() )
        {
            KIO::Job *job = KIO::copy( url, destURL );
	    job->setAutoErrorHandlingEnabled( true );
        }
    }
    delete dlg;
}

void BrowserRun::slotStatResult( KIO::Job *job )
{
    if ( job->error() ) {
        kdDebug(1000) << "BrowserRun::slotStatResult : " << job->errorString() << endl;
        handleError( job );
    } else
        KRun::slotStatResult( job );
}

void BrowserRun::handleError( KIO::Job * job )
{
    // Reuse code in KRun, to benefit from d->m_showingError etc.
    // KHTMLRun and KonqRun reimplement handleError anyway.
    KRun::slotStatResult( job );
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
    m_timer.start( 0, true );
}

bool BrowserRun::isTextExecutable( const QString &serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "application/x-shellscript" );
}

bool BrowserRun::isExecutable( const QString &serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "application/x-executable" ||
             serviceType == "application/x-shellscript" );
}

#include "browserrun.moc"

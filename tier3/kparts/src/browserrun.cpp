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
#include "browserrun_p.h"

#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/scheduler.h>
#include <kio/copyjob.h>
#include <kjobwidgets.h>
#include <klocalizedstring.h>
#include <kshell.h>
#include <kmimetypetrader.h>
#include "browseropenorsavequestion.h"
#include <kprotocolmanager.h>

#include <QTemporaryFile>
#include <QMimeDatabase>
#include <QDebug>
#include <QStandardPaths>
#include <QFileDialog>


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

BrowserRun::BrowserRun( const QUrl & url, const KParts::OpenUrlArguments& args,
                        const KParts::BrowserArguments& browserArgs,
                        KParts::ReadOnlyPart *part, QWidget* window,
                        bool removeReferrer, bool trustedSource, bool hideErrorDialog )
    : KRun( url, window, false /* no GUI */ ),
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

QUrl BrowserRun::url() const
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
        redirectToError(KIO::ERR_MALFORMED_URL, KRun::url().toString());
        return;
    }

    if ( isLocalFile() )  {
      const QString localPath = KRun::url().toLocalFile();
      if (!QFile::exists(localPath)) {
        // qDebug() << localPath << "doesn't exist.";
        redirectToError(KIO::ERR_DOES_NOT_EXIST, localPath);
        return;
      }
    }
  }
  KRun::init();
}

void BrowserRun::scanFile()
{
  // qDebug() << KRun::url();

  // Let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  // Optimization for http/https, findByURL doesn't trust extensions over http.
  QString protocol = KRun::url().scheme();

  if (!KProtocolInfo::proxiedBy(protocol).isEmpty()) {
    QString dummy;
    protocol = KProtocolManager::slaveProtocol(KRun::url(), dummy);
  }

    if (!KRun::url().hasQuery() && !protocol.startsWith(QLatin1String("http"))) {
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForUrl(KRun::url());
        if (!mime.isDefault() || isLocalFile()) {
            // qDebug() << "MIME TYPE is" << mime.name();
            mimeTypeDetermined(mime.name());
            return;
        }
    }

    QMap<QString, QString>& metaData = d->m_args.metaData();
    if ( d->m_part ) {
        const QString proto = d->m_part->url().scheme();

      if (proto == QLatin1String("https") || proto == QLatin1String("webdavs")) {
            metaData.insert(QStringLiteral("main_frame_request"), QStringLiteral("TRUE"));
            metaData.insert(QStringLiteral("ssl_was_in_use"), QStringLiteral("TRUE"));
           // metaData.insert(QStringLiteral("ssl_activate_warnings"), QStringLiteral("TRUE"));
      } else if (proto == QLatin1String("http") || proto == QLatin1String("webdav")) {
           // metaData.insert(QStringLiteral("ssl_activate_warnings"), QStringLiteral("TRUE"));
            metaData.insert(QStringLiteral("ssl_was_in_use"), QStringLiteral("FALSE"));
      }

      // Set the PropagateHttpHeader meta-data if it has not already been set...
        if (!metaData.contains(QStringLiteral("PropagateHttpHeader")))
            metaData.insert(QStringLiteral("PropagateHttpHeader"), QStringLiteral("TRUE"));
  }

  KIO::TransferJob *job;
    if ( d->m_browserArgs.doPost() && KRun::url().scheme().startsWith(QLatin1String("http"))) {
        job = KIO::http_post( KRun::url(), d->m_browserArgs.postData, KIO::HideProgressInfo );
        job->addMetaData(QStringLiteral("content-type"), d->m_browserArgs.contentType() );
    } else {
        job = KIO::get(KRun::url(),
                       d->m_args.reload() ? KIO::Reload : KIO::NoReload,
                       KIO::HideProgressInfo);
  }

    if ( d->m_bRemoveReferrer )
        metaData.remove(QStringLiteral("referrer"));

    job->addMetaData( metaData );
    KJobWidgets::setWindow(job, d->m_window);
  connect( job, SIGNAL(result(KJob*)),
           this, SLOT(slotBrowserScanFinished(KJob*)));
  connect( job, SIGNAL(mimetype(KIO::Job*,QString)),
           this, SLOT(slotBrowserMimetype(KIO::Job*,QString)));
  setJob( job );
}

void BrowserRun::slotBrowserScanFinished(KJob *job)
{
  // qDebug() << job->error();
  if ( job->error() == KIO::ERR_IS_DIRECTORY )
  {
      // It is in fact a directory. This happens when HTTP redirects to FTP.
      // Due to the "protocol doesn't support listing" code in BrowserRun, we
      // assumed it was a file.
      // qDebug() << "It is in fact a directory!";
      // Update our URL in case of a redirection
      KRun::setUrl( static_cast<KIO::TransferJob *>(job)->url() );
      setJob( 0 );
      mimeTypeDetermined(QStringLiteral("inode/directory"));
  }
  else
  {
      KRun::slotScanFinished(job);
  }
}

static QMimeType fixupMimeType (const QString& mimeType, const QString& fileName)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName(mimeType);
    if ((!mime.isValid() || mime.isDefault()) && !fileName.isEmpty()) {
        mime = db.mimeTypeForFile(fileName, QMimeDatabase::MatchExtension);
    }
    return mime;
}

void BrowserRun::slotBrowserMimetype( KIO::Job *_job, const QString &type )
{
    Q_ASSERT( _job == KRun::job() ); Q_UNUSED(_job)
    KIO::TransferJob *job = static_cast<KIO::TransferJob *>(KRun::job());
    // Update our URL in case of a redirection
    //qDebug() << "old URL=" << KRun::url();
    //qDebug() << "new URL=" << job->url();
    setUrl( job->url() );

    if (job->isErrorPage()) {
        d->m_mimeType = type;
        handleError(job);
        setJob( 0 );
    } else {
        // qDebug() << "found" << type << "for" << KRun::url();

        // Suggested filename given by the server (e.g. HTTP content-disposition)
        // When set, we should really be saving instead of embedding
        const QString suggestedFileName = job->queryMetaData(QStringLiteral("content-disposition-filename"));
        setSuggestedFileName(suggestedFileName); // store it (in KRun)
        //qDebug() << "suggestedFileName=" << suggestedFileName;
        d->m_contentDisposition = job->queryMetaData(QStringLiteral("content-disposition-type"));

        const QString modificationTime = job->queryMetaData(QStringLiteral("content-disposition-modification-date"));
        if (!modificationTime.isEmpty()) {
            d->m_args.metaData().insert(QLatin1String("content-disposition-modification-date"), modificationTime);
        }

        QMapIterator<QString,QString> it (job->metaData());
        while (it.hasNext()) {
           it.next();
           if (it.key().startsWith(QLatin1String("ssl_"), Qt::CaseInsensitive))
             d->m_args.metaData().insert(it.key(), it.value());
        }

        // Make a copy to avoid a dead reference
        QString _type = type;
        job->putOnHold();
        setJob( 0 );

        // If the current mime-type is the default mime-type, then attempt to
        // determine the "real" mimetype from the file name.
        QMimeType mime = fixupMimeType(_type, suggestedFileName.isEmpty() ? url().fileName() : suggestedFileName);
        if (mime.isValid() && mime.name() != _type) {
            _type = mime.name();
        }

        mimeTypeDetermined( _type );
    }
}

BrowserRun::NonEmbeddableResult BrowserRun::handleNonEmbeddable(const QString& mimeType)
{
    return handleNonEmbeddable(mimeType, NULL);
}

BrowserRun::NonEmbeddableResult BrowserRun::handleNonEmbeddable(const QString& _mimeType, KService::Ptr* selectedService)
{
    QString mimeType( _mimeType );
    Q_ASSERT( !hasFinished() ); // only come here if the mimetype couldn't be embedded
    // Support for saving remote files.
    if (mimeType != QLatin1String("inode/directory") && // dirs can't be saved
         !KRun::url().isLocalFile() )
    {
        if ( isTextExecutable(mimeType) )
            mimeType = QLatin1String("text/plain"); // view, don't execute
        // ... -> ask whether to save
        BrowserOpenOrSaveQuestion question(d->m_window, KRun::url(), mimeType);
        question.setSuggestedFileName(suggestedFileName());
        if (selectedService)
            question.setFeatures(BrowserOpenOrSaveQuestion::ServiceSelection);
        BrowserOpenOrSaveQuestion::Result res = question.askOpenOrSave();
        if (res == BrowserOpenOrSaveQuestion::Save) {
            save( KRun::url(), suggestedFileName() );
            // qDebug() << "Save: returning Handled";
            setFinished( true );
            return Handled;
        }
        else if (res == BrowserOpenOrSaveQuestion::Cancel) {
            // saving done or canceled
            // qDebug() << "Cancel: returning Handled";
            setFinished( true );
            return Handled;
        }
        else // "Open" chosen (done by KRun::foundMimeType, called when returning NotHandled)
        {
            // If we were in a POST, we can't just pass a URL to an external application.
            // We must save the data to a tempfile first.
            if ( d->m_browserArgs.doPost() )
            {
                // qDebug() << "request comes from a POST, can't pass a URL to another app, need to save";
                d->m_mimeType = mimeType;
                QString extension;
                QString fileName = suggestedFileName().isEmpty() ? KRun::url().fileName() : suggestedFileName();
                int extensionPos = fileName.lastIndexOf(QLatin1Char('.'));
                if ( extensionPos != -1 )
                    extension = fileName.mid( extensionPos ); // keep the '.'
                QTemporaryFile tempFile(QDir::tempPath() + QLatin1Char('/') + QCoreApplication::applicationName() + QLatin1String("XXXXXX") + extension);
                tempFile.setAutoRemove(false);
                tempFile.open();
                QUrl destURL = QUrl::fromLocalFile(tempFile.fileName());
                KIO::Job *job = KIO::file_copy( KRun::url(), destURL, 0600, KIO::Overwrite );
                KJobWidgets::setWindow(job, d->m_window);
                connect( job, SIGNAL(result(KJob*)),
                         this, SLOT(slotCopyToTempFileResult(KJob*)) );
                return Delayed; // We'll continue after the job has finished
            }
            if (selectedService && question.selectedService()) {
                *selectedService = question.selectedService();
                // KRun will use this when starting an app
                KRun::setPreferredService(question.selectedService()->desktopEntryName());
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

    KIO::Scheduler::publishSlaveOnHold(); // publish any slave on hold so it can be reused.
    return NotHandled;
}

//static
bool BrowserRun::allowExecution( const QString &mimeType, const QUrl &url )
{
    if ( !KRun::isExecutable( mimeType ) )
      return true;

    if ( !url.isLocalFile() ) // Don't permit to execute remote files
        return false;

    return ( KMessageBox::warningContinueCancel( 0,
                                                 i18n("Do you really want to execute '%1'?", url.toDisplayString()),
    i18n("Execute File?"), KGuiItem(i18n("Execute")) ) == KMessageBox::Continue );
}

//static, deprecated
#ifndef KDE_NO_DEPRECATED
BrowserRun::AskSaveResult BrowserRun::askSave( const QUrl & url, KService::Ptr offer, const QString& mimeType, const QString & suggestedFileName )
{
    Q_UNUSED(offer);
    BrowserOpenOrSaveQuestion question(0, url, mimeType);
    question.setSuggestedFileName(suggestedFileName);
    const BrowserOpenOrSaveQuestion::Result result = question.askOpenOrSave();
    return result == BrowserOpenOrSaveQuestion::Save ? Save
        : BrowserOpenOrSaveQuestion::Open ? Open
        : Cancel;
}
#endif

//static, deprecated
#ifndef KDE_NO_DEPRECATED
BrowserRun::AskSaveResult BrowserRun::askEmbedOrSave( const QUrl & url, const QString& mimeType, const QString & suggestedFileName, int flags )
{
    BrowserOpenOrSaveQuestion question(0, url, mimeType);
    question.setSuggestedFileName(suggestedFileName);
    const BrowserOpenOrSaveQuestion::Result result = question.askEmbedOrSave(flags);
    return result == BrowserOpenOrSaveQuestion::Save ? Save
        : BrowserOpenOrSaveQuestion::Embed ? Open
        : Cancel;
}
#endif

// Default implementation, overridden in KHTMLRun
void BrowserRun::save( const QUrl & url, const QString & suggestedFileName )
{
    saveUrl(url, suggestedFileName, d->m_window, d->m_args);
}

// static
void BrowserRun::simpleSave( const QUrl & url, const QString & suggestedFileName,
                             QWidget* window )
{
    saveUrl(url, suggestedFileName, window, KParts::OpenUrlArguments());
}

void KParts::BrowserRun::saveUrl(const QUrl & url, const QString & suggestedFileName,
                                 QWidget* window, const KParts::OpenUrlArguments& args)
{
    // DownloadManager <-> konqueror integration
    // find if the integration is enabled
    // the empty key  means no integration
    // only use the downloadmanager for non-local urls
    if ( !url.isLocalFile() )
    {
        KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("konquerorrc"), KConfig::NoGlobals)->group("HTML Settings");
        QString downloadManger = cfg.readPathEntry("DownloadManager", QString());
        if (!downloadManger.isEmpty())
        {
            // then find the download manager location
            // qDebug() << "Using: "<<downloadManger <<" as Download Manager";
            QString cmd = QStandardPaths::findExecutable(downloadManger);
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
                cmd += QLatin1Char(' ') + KShell::quoteArg(url.toString());
                if ( !suggestedFileName.isEmpty() )
                    cmd += QLatin1Char(' ') + KShell::quoteArg(suggestedFileName);

                // qDebug() << "Calling command" << cmd;
                // slave is already on hold (slotBrowserMimetype())
                KIO::Scheduler::publishSlaveOnHold();
                KRun::runCommand(cmd, window);
                return;
            }
        }
    }

    // no download manager available, let's do it ourself
    QFileDialog *dlg = new QFileDialog(window);
    dlg->setAcceptMode(QFileDialog::AcceptSave);
    dlg->setWindowTitle(i18n("Save As"));
    dlg->setConfirmOverwrite(true);

    QString name;
    if (!suggestedFileName.isEmpty())
        name = suggestedFileName;
    else
        name = url.fileName(); // can be empty, e.g. in case http://www.kde.org/

    dlg->selectFile(name);
    if (dlg->exec())
    {
        QUrl destURL(dlg->selectedUrls().first());
        if (destURL.isValid())
        {
            saveUrlUsingKIO(url, destURL, window, args.metaData());
        }
    }
    delete dlg;
}

void BrowserRun::saveUrlUsingKIO(const QUrl & srcUrl, const QUrl & destUrl,
                                 QWidget* window, const QMap<QString, QString> &metaData)
{
    KIO::FileCopyJob *job = KIO::file_copy(srcUrl, destUrl, -1, KIO::Overwrite);

    const QString modificationTime = metaData[QLatin1String("content-disposition-modification-date")];
    if (!modificationTime.isEmpty()) {
        job->setModificationTime(QDateTime::fromString(modificationTime, Qt::RFC2822Date));
    }
    job->setMetaData(metaData);
    job->addMetaData(QStringLiteral("MaxCacheSize"), QStringLiteral("0")); // Don't store in http cache.
    job->addMetaData(QStringLiteral("cache"), QStringLiteral("cache")); // Use entry from cache if available.
    KJobWidgets::setWindow(job, window);
    job->ui()->setAutoErrorHandlingEnabled( true );
    new DownloadJobWatcher(job, metaData);
}

void BrowserRun::handleError( KJob * job )
{
    if ( !job ) { // Shouldn't happen
        qWarning() << "handleError called with job=0! hideErrorDialog=" << d->m_bHideErrorDialog;
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
    KRun::handleError( job );
}

// static
QUrl BrowserRun::makeErrorUrl(int error, const QString& errorText, const QUrl& initialUrl)
{
    /*
     * The format of the error:/ URL is error:/?query#url,
     * where two variables are passed in the query:
     * error = int kio error code, errText = QString error text from kio
     * The sub-url is the URL that we were trying to open.
     */
    QUrl newURL(QString::fromLatin1("error:/?error=%1&errText=%2")
                .arg(error)
                .arg(QString::fromUtf8(QUrl::toPercentEncoding(errorText))));

    QString cleanedOrigUrl = initialUrl.toString();
    QUrl runURL(cleanedOrigUrl);
    if (runURL.isValid()) {
        runURL.setPassword(QString()); // don't put the password in the error URL
        cleanedOrigUrl = runURL.toString();
    }

    newURL.setFragment(cleanedOrigUrl);
    return newURL;

    // The kde3 approach broke with invalid urls, now that they become empty in qt4.
    //QList<QUrl> lst;
    //lst << newURL << runURL;
    //return KUrl::join(lst);
}

void BrowserRun::redirectToError( int error, const QString& errorText )
{
    /**
     * To display this error in KHTMLPart instead of inside a dialog box,
     * we tell konq that the mimetype is text/html, and we redirect to
     * an error:/ URL that sends the info to khtml.
     */
    KRun::setUrl(makeErrorUrl(error, errorText, url()));
    setJob( 0 );
    mimeTypeDetermined(QStringLiteral("text/html"));
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
}

bool BrowserRun::isTextExecutable( const QString &mimeType )
{
    return ( mimeType == QLatin1String("application/x-desktop") ||
             mimeType == QLatin1String("application/x-shellscript") );
}

bool BrowserRun::hideErrorDialog() const
{
    return d->m_bHideErrorDialog;
}

QString BrowserRun::contentDisposition() const
{
    return d->m_contentDisposition;
}

bool BrowserRun::serverSuggestsSave() const
{
    // RfC 2183, section 2.8:
    // Unrecognized disposition types should be treated as `attachment'.
    return !contentDisposition().isEmpty() && (contentDisposition() != QLatin1String("inline"));
}

KParts::OpenUrlArguments& KParts::BrowserRun::arguments()
{
    return d->m_args;
}

KParts::BrowserArguments& KParts::BrowserRun::browserArguments()
{
    return d->m_browserArgs;
}

#include "moc_browserrun.cpp"
#include "moc_browserrun_p.cpp"

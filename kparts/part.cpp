/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "part.h"
#include <kprotocolinfo.h>
#include "event.h"
#include "plugin.h"
#include "mainwindow.h"
#include "partmanager.h"
#include "browserextension.h"

#include <QApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <qtemporaryfile.h>
#include <QtCore/QPoint>
#include <qmimedatabase.h>

#include <kdirnotify.h>
#include <kfiledialog.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <qurlpathinfo.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <kdebug.h>
#include <kiconloader.h>


using namespace KParts;

namespace KParts
{

class PartBasePrivate
{
public:
    Q_DECLARE_PUBLIC(PartBase)

    PartBasePrivate(PartBase *q): q_ptr(q)
    {
        m_pluginLoadingMode = PartBase::LoadPlugins;
        m_pluginInterfaceVersion = 0;
        m_obj = 0;
    }

    virtual ~PartBasePrivate()
    {
    }

    KComponentData m_componentData;
    PartBase *q_ptr;
    PartBase::PluginLoadingMode m_pluginLoadingMode;
    int m_pluginInterfaceVersion;
    QObject *m_obj;
};

class PartPrivate: public PartBasePrivate
{
public:
    Q_DECLARE_PUBLIC(Part)

    PartPrivate(Part *q)
        : PartBasePrivate(q),
          m_iconLoader(0),
          m_bSelectable(true),
          m_autoDeleteWidget(true),
          m_autoDeletePart(true),
          m_manager(0)
    {
    }

    ~PartPrivate()
    {
    }

    KIconLoader* m_iconLoader;
    bool m_bSelectable;
    bool m_autoDeleteWidget;
    bool m_autoDeletePart;
    PartManager * m_manager;
    QPointer<QWidget> m_widget;
};

}

PartBase::PartBase()
    : d_ptr(new PartBasePrivate(this))
{
}

PartBase::PartBase(PartBasePrivate &dd)
    : d_ptr(&dd)
{
}

PartBase::~PartBase()
{
    delete d_ptr;
}

void PartBase::setPartObject( QObject *obj )
{
    Q_D(PartBase);

    d->m_obj = obj;
}

QObject *PartBase::partObject() const
{
    Q_D(const PartBase);

    return d->m_obj;
}

KComponentData PartBase::componentData() const
{
    Q_D(const PartBase);
    return d->m_componentData;
}

void PartBase::setComponentData(const KComponentData &componentData)
{
    setComponentData(componentData, true);
}

void PartBase::setComponentData(const KComponentData &componentData, bool bLoadPlugins)
{
    Q_D(PartBase);

    d->m_componentData = componentData;
    KXMLGUIClient::setComponentName(componentData.componentName(), componentData.aboutData()->programName());
    KLocalizedString::insertCatalog(componentData.catalogName());
    if (bLoadPlugins) {
        loadPlugins(d->m_obj, this, componentData);
    }
}

void PartBase::loadPlugins(QObject *parent, KXMLGUIClient *parentGUIClient, const KComponentData &instance)
{
    Q_D(PartBase);

    if( d->m_pluginLoadingMode != DoNotLoadPlugins )
        Plugin::loadPlugins( parent, parentGUIClient, instance, d->m_pluginLoadingMode == LoadPlugins, d->m_pluginInterfaceVersion );
}

void PartBase::setPluginLoadingMode( PluginLoadingMode loadingMode )
{
    Q_D(PartBase);

    d->m_pluginLoadingMode = loadingMode;
}

void KParts::PartBase::setPluginInterfaceVersion( int version )
{
    Q_D(PartBase);

    d->m_pluginInterfaceVersion = version;
}

Part::Part( QObject *parent )
    : QObject( parent ), PartBase( *new PartPrivate(this) )
{
    PartBase::setPartObject( this );
}

Part::Part(PartPrivate &dd, QObject *parent)
    : QObject( parent ), PartBase( dd )
{
    PartBase::setPartObject( this );
}

Part::~Part()
{
    Q_D(Part);

    //kDebug(1000) << this;

    if ( d->m_widget )
    {
        // We need to disconnect first, to avoid calling it !
        disconnect( d->m_widget, SIGNAL(destroyed()),
                    this, SLOT(slotWidgetDestroyed()) );
    }

    if ( d->m_manager )
        d->m_manager->removePart(this);

    if ( d->m_widget && d->m_autoDeleteWidget )
    {
        kDebug(1000) << "deleting widget" << d->m_widget << d->m_widget->objectName();
        delete static_cast<QWidget*>(d->m_widget);
    }

    delete d->m_iconLoader;
}

void Part::embed( QWidget * parentWidget )
{
    if ( widget() )
    {
        widget()->setParent( parentWidget, 0 );
        widget()->setGeometry( 0, 0, widget()->width(), widget()->height() );
        widget()->show();
    }
}

QWidget *Part::widget()
{
    Q_D(Part);

    return d->m_widget;
}

void Part::setAutoDeleteWidget(bool autoDeleteWidget)
{
    Q_D(Part);
    d->m_autoDeleteWidget = autoDeleteWidget;
}

void Part::setAutoDeletePart(bool autoDeletePart)
{
    Q_D(Part);
    d->m_autoDeletePart = autoDeletePart;
}



KIconLoader* Part::iconLoader()
{
    Q_D(Part);

    if (!d->m_iconLoader) {
        d->m_iconLoader = new KIconLoader(componentName());
    }
    return d->m_iconLoader;
}

void Part::setManager( PartManager *manager )
{
    Q_D(Part);

    d->m_manager = manager;
}

PartManager *Part::manager() const
{
    Q_D(const Part);

    return d->m_manager;
}

Part *Part::hitTest( QWidget *widget, const QPoint & )
{
    Q_D(Part);

    if ( (QWidget *)d->m_widget != widget )
        return 0;

    return this;
}

void Part::setWidget( QWidget *widget )
{
    Q_D(Part);
    d->m_widget = widget;
    connect( d->m_widget, SIGNAL(destroyed()),
             this, SLOT(slotWidgetDestroyed()), Qt::UniqueConnection );
}

void Part::setSelectable( bool selectable )
{
    Q_D(Part);

    d->m_bSelectable = selectable;
}

bool Part::isSelectable() const
{
    Q_D(const Part);

    return d->m_bSelectable;
}

void Part::customEvent( QEvent *ev )
{
    if ( PartActivateEvent::test( ev ) )
    {
        partActivateEvent( static_cast<PartActivateEvent *>(ev) );
        return;
    }

    if ( PartSelectEvent::test( ev ) )
    {
        partSelectEvent( static_cast<PartSelectEvent *>(ev) );
        return;
    }

    if ( GUIActivateEvent::test( ev ) )
    {
        guiActivateEvent( static_cast<GUIActivateEvent *>(ev) );
        return;
    }

    QObject::customEvent( ev );
}

void Part::partActivateEvent( PartActivateEvent * )
{
}

void Part::partSelectEvent( PartSelectEvent * )
{
}

void Part::guiActivateEvent( GUIActivateEvent * )
{
}

QWidget *Part::hostContainer( const QString &containerName )
{
    if ( !factory() )
        return 0;

    return factory()->container( containerName, this );
}

void Part::slotWidgetDestroyed()
{
    Q_D(Part);

    d->m_widget = 0;
    if (d->m_autoDeletePart) {
        kDebug(1000) << "deleting part" << objectName();
        delete this; // ouch, this should probably be deleteLater()
    }
}

void Part::loadPlugins()
{
    PartBase::loadPlugins(this, this, componentData());
}

//////////////////////////////////////////////////

namespace KParts
{

class ReadOnlyPartPrivate: public PartPrivate
{
public:
    Q_DECLARE_PUBLIC(ReadOnlyPart)

    ReadOnlyPartPrivate(ReadOnlyPart *q): PartPrivate(q)
    {
        m_job = 0;
        m_statJob = 0;
        m_uploadJob = 0;
        m_showProgressInfo = true;
        m_saveOk = false;
        m_waitForSave = false;
        m_duringSaveAs = false;
        m_bTemp = false;
        m_bAutoDetectedMime = false;
    }

    ~ReadOnlyPartPrivate()
    {
    }

    void _k_slotJobFinished( KJob * job );
    void _k_slotStatJobFinished(KJob * job);
    void _k_slotGotMimeType(KIO::Job *job, const QString &mime);
    bool openLocalFile();
    void openRemoteFile();

    KIO::FileCopyJob * m_job;
    KIO::StatJob * m_statJob;
    KIO::FileCopyJob * m_uploadJob;
    QUrl m_originalURL; // for saveAs
    QString m_originalFilePath; // for saveAs
    bool m_showProgressInfo : 1;
    bool m_saveOk : 1;
    bool m_waitForSave : 1;
    bool m_duringSaveAs : 1;

    /**
     * If @p true, @p m_file is a temporary file that needs to be deleted later.
     */
    bool m_bTemp: 1;

    // whether the mimetype in the arguments was detected by the part itself
    bool m_bAutoDetectedMime : 1;

    /**
     * Remote (or local) url - the one displayed to the user.
     */
    QUrl m_url;

    /**
     * Local file - the only one the part implementation should deal with.
     */
    QString m_file;

    OpenUrlArguments m_arguments;
};

class ReadWritePartPrivate: public ReadOnlyPartPrivate
{
public:
    Q_DECLARE_PUBLIC(ReadWritePart)

    ReadWritePartPrivate(ReadWritePart *q): ReadOnlyPartPrivate(q)
    {
        m_bModified = false;
        m_bReadWrite = true;
        m_bClosing = false;
    }

    void _k_slotUploadFinished( KJob * job );

    void prepareSaving();

    bool m_bModified;
    bool m_bReadWrite;
    bool m_bClosing;
    QEventLoop m_eventLoop;
};

}

ReadOnlyPart::ReadOnlyPart( QObject *parent )
    : Part( *new ReadOnlyPartPrivate(this), parent )
{
}

ReadOnlyPart::ReadOnlyPart( ReadOnlyPartPrivate &dd, QObject *parent )
    : Part( dd, parent )
{
}

ReadOnlyPart::~ReadOnlyPart()
{
    ReadOnlyPart::closeUrl();
}

QUrl ReadOnlyPart::url() const
{
    Q_D(const ReadOnlyPart);

    return d->m_url;
}

void ReadOnlyPart::setUrl(const QUrl &url)
{
    Q_D(ReadOnlyPart);

    d->m_url = url;
    emit urlChanged( url );
}

QString ReadOnlyPart::localFilePath() const
{
    Q_D(const ReadOnlyPart);

    return d->m_file;
}

void ReadOnlyPart::setLocalFilePath( const QString &localFilePath )
{
    Q_D(ReadOnlyPart);

    d->m_file = localFilePath;
}

#ifndef KDE_NO_DEPRECATED
bool ReadOnlyPart::isLocalFileTemporary() const
{
    Q_D(const ReadOnlyPart);

    return d->m_bTemp;
}
#endif

#ifndef KDE_NO_DEPRECATED
void ReadOnlyPart::setLocalFileTemporary( bool temp )
{
    Q_D(ReadOnlyPart);

    d->m_bTemp = temp;
}
#endif

void ReadOnlyPart::setProgressInfoEnabled( bool show )
{
    Q_D(ReadOnlyPart);

    d->m_showProgressInfo = show;
}

bool ReadOnlyPart::isProgressInfoEnabled() const
{
    Q_D(const ReadOnlyPart);

    return d->m_showProgressInfo;
}

#ifndef KDE_NO_COMPAT
void ReadOnlyPart::showProgressInfo( bool show )
{
    Q_D(ReadOnlyPart);

    d->m_showProgressInfo = show;
}
#endif

bool ReadOnlyPart::openUrl( const QUrl &url )
{
    Q_D(ReadOnlyPart);

    if ( !url.isValid() )
        return false;
    if (d->m_bAutoDetectedMime) {
        d->m_arguments.setMimeType(QString());
        d->m_bAutoDetectedMime = false;
    }
    OpenUrlArguments args = d->m_arguments;
    if ( !closeUrl() )
        return false;
    d->m_arguments = args;
    setUrl(url);

    d->m_file.clear();

    if (d->m_url.isLocalFile()) {
        d->m_file = d->m_url.toLocalFile();
        return d->openLocalFile();
    } else if (KProtocolInfo::protocolClass(url.scheme()) == ":local") {
        // Maybe we can use a "local path", to avoid a temp copy?
        KIO::JobFlags flags = d->m_showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
        d->m_statJob = KIO::mostLocalUrl(d->m_url, flags);
        d->m_statJob->ui()->setWindow( widget() ? widget()->topLevelWidget() : 0 );
        connect(d->m_statJob, SIGNAL(result(KJob*)), this, SLOT(_k_slotStatJobFinished(KJob*)));
        return true;
    } else {
        d->openRemoteFile();
        return true;
    }
}

bool ReadOnlyPart::openFile()
{
    kWarning(1000) << "Default implementation of ReadOnlyPart::openFile called!"
                   << metaObject()->className() << "should reimplement either openUrl or openFile.";
    return false;
}

bool ReadOnlyPartPrivate::openLocalFile()
{
    Q_Q(ReadOnlyPart);
    emit q->started( 0 );
    m_bTemp = false;
    // set the mimetype only if it was not already set (for example, by the host application)
    if (m_arguments.mimeType().isEmpty()) {
        // get the mimetype of the file
        // using findByUrl() to avoid another string -> url conversion
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForUrl(m_url);
        if (!mime.isDefault()) {
            m_arguments.setMimeType(mime.name());
            m_bAutoDetectedMime = true;
        }
    }
    const bool ret = q->openFile();
    if (ret) {
        emit q->setWindowCaption(m_url.toDisplayString());
        emit q->completed();
    } else {
        emit q->canceled(QString());
    }
    return ret;
}

void ReadOnlyPartPrivate::openRemoteFile()
{
    Q_Q(ReadOnlyPart);
    m_bTemp = true;
    // Use same extension as remote file. This is important for mimetype-determination (e.g. koffice)
    QString fileName = QUrlPathInfo(m_url).fileName();
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.completeSuffix();
    QString extension;
    if (!ext.isEmpty() && !m_url.hasQuery()) // not if the URL has a query, e.g. cgi.pl?something
        extension = '.'+ext; // keep the '.'
    QTemporaryFile tempFile(QDir::tempPath() + QLatin1Char('/') + q->componentData().componentName() + QLatin1String("XXXXXX") + extension);
    tempFile.setAutoRemove(false);
    tempFile.open();
    m_file = tempFile.fileName();

    QUrl destURL = QUrl::fromLocalFile(m_file);
    KIO::JobFlags flags = m_showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
    flags |= KIO::Overwrite;
    m_job = KIO::file_copy(m_url, destURL, 0600, flags);
    m_job->ui()->setWindow(q->widget() ? q->widget()->topLevelWidget() : 0);
    emit q->started(m_job);
    QObject::connect(m_job, SIGNAL(result(KJob*)), q, SLOT(_k_slotJobFinished(KJob*)));
    QObject::connect(m_job, SIGNAL(mimetype(KIO::Job*,QString)),
                     q, SLOT(_k_slotGotMimeType(KIO::Job*,QString)));
}

void ReadOnlyPart::abortLoad()
{
    Q_D(ReadOnlyPart);

    if ( d->m_statJob ) {
        //kDebug(1000) << "Aborting job" << d->m_statJob;
        d->m_statJob->kill();
        d->m_statJob = 0;
    }
    if ( d->m_job ) {
        //kDebug(1000) << "Aborting job" << d->m_job;
        d->m_job->kill();
        d->m_job = 0;
    }
}

bool ReadOnlyPart::closeUrl()
{
    Q_D(ReadOnlyPart);

    abortLoad(); //just in case

    d->m_arguments = KParts::OpenUrlArguments();

    if ( d->m_bTemp )
    {
        QFile::remove( d->m_file );
        d->m_bTemp = false;
    }
    // It always succeeds for a read-only part,
    // but the return value exists for reimplementations
    // (e.g. pressing cancel for a modified read-write part)
    return true;
}

void ReadOnlyPartPrivate::_k_slotStatJobFinished(KJob * job)
{
    Q_ASSERT(job == m_statJob);
    m_statJob = 0;

    // We could emit canceled on error, but we haven't even emitted started yet,
    // this could maybe confuse some apps? So for now we'll just fallback to KIO::get
    // and error again. Well, maybe this even helps with wrong stat results.
    if (!job->error()) {
        const QUrl localUrl = static_cast<KIO::StatJob*>(job)->mostLocalUrl();
        if (localUrl.isLocalFile()) {
            m_file = localUrl.toLocalFile();
            (void)openLocalFile();
            return;
        }
    }
    openRemoteFile();
}

void ReadOnlyPartPrivate::_k_slotJobFinished( KJob * job )
{
    Q_Q(ReadOnlyPart);

    assert( job == m_job );
    m_job = 0;
    if (job->error())
        emit q->canceled( job->errorString() );
    else
    {
        if ( q->openFile() ) {
            emit q->setWindowCaption(m_url.toDisplayString());
            emit q->completed();
        } else emit q->canceled(QString());
    }
}

void ReadOnlyPartPrivate::_k_slotGotMimeType(KIO::Job *job, const QString &mime)
{
    kDebug(1000) << mime;
    Q_ASSERT(job == m_job); Q_UNUSED(job)
    // set the mimetype only if it was not already set (for example, by the host application)
    if (m_arguments.mimeType().isEmpty()) {
        m_arguments.setMimeType(mime);
        m_bAutoDetectedMime = true;
    }
}

void ReadOnlyPart::guiActivateEvent( GUIActivateEvent * event )
{
    Q_D(ReadOnlyPart);

    if (event->activated())
    {
        if (!d->m_url.isEmpty())
        {
            kDebug(1000) << d->m_url;
            emit setWindowCaption(d->m_url.toDisplayString());
        } else emit setWindowCaption( "" );
    }
}

bool ReadOnlyPart::openStream( const QString& mimeType, const QUrl & url )
{
    Q_D(ReadOnlyPart);

    OpenUrlArguments args = d->m_arguments;
    if ( !closeUrl() )
        return false;
    d->m_arguments = args;
    setUrl( url );
    return doOpenStream( mimeType );
}

bool ReadOnlyPart::writeStream( const QByteArray& data )
{
    return doWriteStream( data );
}

bool ReadOnlyPart::closeStream()
{
    return doCloseStream();
}

BrowserExtension* ReadOnlyPart::browserExtension() const
{
    return findChild<KParts::BrowserExtension *>();
}

void KParts::ReadOnlyPart::setArguments(const OpenUrlArguments& arguments)
{
    Q_D(ReadOnlyPart);
    d->m_arguments = arguments;
    d->m_bAutoDetectedMime = arguments.mimeType().isEmpty();
}

OpenUrlArguments KParts::ReadOnlyPart::arguments() const
{
    Q_D(const ReadOnlyPart);
    return d->m_arguments;
}

//////////////////////////////////////////////////


ReadWritePart::ReadWritePart( QObject *parent )
    : ReadOnlyPart( *new ReadWritePartPrivate(this), parent )
{
}

ReadWritePart::~ReadWritePart()
{
    // parent destructor will delete temp file
    // we can't call our own closeUrl() here, because
    // "cancel" wouldn't cancel anything. We have to assume
    // the app called closeUrl() before destroying us.
}

void ReadWritePart::setReadWrite( bool readwrite )
{
    Q_D(ReadWritePart);

    // Perhaps we should check isModified here and issue a warning if true
    d->m_bReadWrite = readwrite;
}

void ReadWritePart::setModified( bool modified )
{
    Q_D(ReadWritePart);

    kDebug(1000) << "setModified(" << (modified ? "true" : "false") << ")";
    if ( !d->m_bReadWrite && modified )
    {
        kError(1000) << "Can't set a read-only document to 'modified' !" << endl;
        return;
    }
    d->m_bModified = modified;
}

void ReadWritePart::setModified()
{
    setModified( true );
}

bool ReadWritePart::queryClose()
{
    Q_D(ReadWritePart);

    if ( !isReadWrite() || !isModified() )
        return true;

    QString docName = QUrlPathInfo(url()).fileName();
    if (docName.isEmpty()) docName = i18n( "Untitled" );

    QWidget *parentWidget=widget();
    if(!parentWidget) parentWidget=QApplication::activeWindow();

    int res = KMessageBox::warningYesNoCancel( parentWidget,
                                               i18n( "The document \"%1\" has been modified.\n"
                                                     "Do you want to save your changes or discard them?" ,  docName ),
                                               i18n( "Close Document" ), KStandardGuiItem::save(), KStandardGuiItem::discard() );

    bool abortClose=false;
    bool handled=false;

    switch(res) {
    case KMessageBox::Yes :
        sigQueryClose(&handled,&abortClose);
        if (!handled)
        {
            if (d->m_url.isEmpty())
            {
                QUrl url = KFileDialog::getSaveUrl(QUrl(), QString(), parentWidget);
                if (url.isEmpty())
                    return false;

                saveAs( url );
            }
            else
            {
                save();
            }
        } else if (abortClose) return false;
        return waitSaveComplete();
    case KMessageBox::No :
        return true;
    default : // case KMessageBox::Cancel :
        return false;
    }
}

bool ReadWritePart::closeUrl()
{
    abortLoad(); //just in case
    if ( isReadWrite() && isModified() )
    {
        if (!queryClose())
            return false;
    }
    // Not modified => ok and delete temp file.
    return ReadOnlyPart::closeUrl();
}

bool ReadWritePart::closeUrl( bool promptToSave )
{
    return promptToSave ? closeUrl() : ReadOnlyPart::closeUrl();
}

bool ReadWritePart::save()
{
    Q_D(ReadWritePart);

    d->m_saveOk = false;
    if ( d->m_file.isEmpty() ) // document was created empty
        d->prepareSaving();
    if( saveFile() )
        return saveToUrl();
    else
        emit canceled(QString());
    return false;
}

bool ReadWritePart::saveAs(const QUrl & url)
{
    Q_D(ReadWritePart);

    if (!url.isValid())
    {
        kError(1000) << "saveAs: Malformed URL " << url << endl;
        return false;
    }
    d->m_duringSaveAs = true;
    d->m_originalURL = d->m_url;
    d->m_originalFilePath = d->m_file;
    d->m_url = url; // Store where to upload in saveToURL
    d->prepareSaving();
    bool result = save(); // Save local file and upload local file
    if (result) {
        emit urlChanged(d->m_url);
        emit setWindowCaption(d->m_url.toDisplayString());
    } else {
        d->m_url = d->m_originalURL;
        d->m_file = d->m_originalFilePath;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
    }

    return result;
}

// Set m_file correctly for m_url
void ReadWritePartPrivate::prepareSaving()
{
    // Local file
    if ( m_url.isLocalFile() )
    {
        if ( m_bTemp ) // get rid of a possible temp file first
        {              // (happens if previous url was remote)
            QFile::remove( m_file );
            m_bTemp = false;
        }
        m_file = m_url.toLocalFile();
    }
    else
    { // Remote file
        // We haven't saved yet, or we did but locally - provide a temp file
        if ( m_file.isEmpty() || !m_bTemp )
        {
            QTemporaryFile tempFile;
            tempFile.setAutoRemove(false);
            tempFile.open();
            m_file = tempFile.fileName();
            m_bTemp = true;
        }
        // otherwise, we already had a temp file
    }
}

bool ReadWritePart::saveToUrl()
{
    Q_D(ReadWritePart);

    if ( d->m_url.isLocalFile() )
    {
        setModified( false );
        emit completed();
        // if m_url is a local file there won't be a temp file -> nothing to remove
        assert( !d->m_bTemp );
        d->m_saveOk = true;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
        return true; // Nothing to do
    }
    else
    {
        if (d->m_uploadJob)
        {
            QFile::remove(d->m_uploadJob->srcUrl().toLocalFile());
            d->m_uploadJob->kill();
            d->m_uploadJob = 0;
        }
        QTemporaryFile *tempFile = new QTemporaryFile();
        tempFile->open();
        QString uploadFile = tempFile->fileName();
        delete tempFile;
        QUrl uploadUrl = QUrl::fromLocalFile(uploadFile);
        // Create hardlink
        if (::link(QFile::encodeName(d->m_file), QFile::encodeName(uploadFile)) != 0)
        {
            // Uh oh, some error happened.
            return false;
        }
        d->m_uploadJob = KIO::file_move( uploadUrl, d->m_url, -1, KIO::Overwrite );
        d->m_uploadJob->ui()->setWindow( widget() ? widget()->topLevelWidget() : 0 );
        connect( d->m_uploadJob, SIGNAL(result(KJob*)), this, SLOT(_k_slotUploadFinished(KJob*)) );
        return true;
    }
}

void ReadWritePartPrivate::_k_slotUploadFinished( KJob * )
{
    Q_Q(ReadWritePart);

    if (m_uploadJob->error())
    {
        QFile::remove(m_uploadJob->srcUrl().toLocalFile());
        QString error = m_uploadJob->errorString();
        m_uploadJob = 0;
        if (m_duringSaveAs) {
            q->setUrl(m_originalURL);
            m_file = m_originalFilePath;
        }
        emit q->canceled( error );
    }
    else
    {
        QUrlPathInfo dirUrlInfo( m_url );
        dirUrlInfo.setPath(dirUrlInfo.directory());
        ::org::kde::KDirNotify::emitFilesAdded(dirUrlInfo.url());

        m_uploadJob = 0;
        q->setModified( false );
        emit q->completed();
        m_saveOk = true;
    }
    m_duringSaveAs = false;
    m_originalURL = QUrl();
    m_originalFilePath.clear();
    if (m_waitForSave) {
        m_eventLoop.quit();
    }
}

bool ReadWritePart::isReadWrite() const
{
    Q_D(const ReadWritePart);

    return d->m_bReadWrite;
}

bool ReadWritePart::isModified() const
{
    Q_D(const ReadWritePart);

    return d->m_bModified;
}

bool ReadWritePart::waitSaveComplete()
{
    Q_D(ReadWritePart);

    if (!d->m_uploadJob)
        return d->m_saveOk;

    d->m_waitForSave = true;

    d->m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    d->m_waitForSave = false;

    return d->m_saveOk;
}

////

class KParts::OpenUrlArgumentsPrivate : public QSharedData
{
public:
    OpenUrlArgumentsPrivate()
        : reload(false),
          actionRequestedByUser(true),
          xOffset(0),
          yOffset(0),
          mimeType(),
          metaData()
    {}
    bool reload;
    bool actionRequestedByUser;
    int xOffset;
    int yOffset;
    QString mimeType;
    QMap<QString, QString> metaData;
};

KParts::OpenUrlArguments::OpenUrlArguments()
    : d(new OpenUrlArgumentsPrivate)
{
}

KParts::OpenUrlArguments::OpenUrlArguments(const OpenUrlArguments &other)
    : d(other.d)
{
}

KParts::OpenUrlArguments & KParts::OpenUrlArguments::operator=( const OpenUrlArguments &other)
{
    d = other.d;
    return *this;
}

KParts::OpenUrlArguments::~OpenUrlArguments()
{
}

bool KParts::OpenUrlArguments::reload() const
{
    return d->reload;
}

void KParts::OpenUrlArguments::setReload(bool b)
{
    d->reload = b;
}

int KParts::OpenUrlArguments::xOffset() const
{
    return d->xOffset;
}

void KParts::OpenUrlArguments::setXOffset(int x)
{
    d->xOffset = x;
}

int KParts::OpenUrlArguments::yOffset() const
{
    return d->yOffset;
}

void KParts::OpenUrlArguments::setYOffset(int y)
{
    d->yOffset = y;
}

QString KParts::OpenUrlArguments::mimeType() const
{
    return d->mimeType;
}

void KParts::OpenUrlArguments::setMimeType(const QString& mime)
{
    d->mimeType = mime;
}

QMap<QString, QString> & KParts::OpenUrlArguments::metaData()
{
    return d->metaData;
}

const QMap<QString, QString> & KParts::OpenUrlArguments::metaData() const
{
    return d->metaData;
}

bool KParts::OpenUrlArguments::actionRequestedByUser() const
{
    return d->actionRequestedByUser;
}

void KParts::OpenUrlArguments::setActionRequestedByUser(bool userRequested)
{
    d->actionRequestedByUser = userRequested;
}

#include "moc_part.cpp"

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kparts/part.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <q3pointarray.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qtextstream.h>

#include <kdirnotify_stub.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kxmlguifactory.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <kdebug.h>

template class Q3PtrList<KXMLGUIClient>;

using namespace KParts;

namespace KParts
{

class PartBasePrivate
{
public:
  PartBasePrivate()
  {
      m_pluginLoadingMode = PartBase::LoadPlugins;
  }
  ~PartBasePrivate()
  {
  }
  PartBase::PluginLoadingMode m_pluginLoadingMode;
};

class PartPrivate
{
public:
  PartPrivate()
  {
    m_bSelectable = true;
  }
  ~PartPrivate()
  {
  }

  bool m_bSelectable;
};
}

PartBase::PartBase()
{
  d = new PartBasePrivate;
  m_obj = 0L;
}

PartBase::~PartBase()
{
  delete d;
}

void PartBase::setPartObject( QObject *obj )
{
  m_obj = obj;
}

QObject *PartBase::partObject() const
{
  return m_obj;
}

void PartBase::setInstance( KInstance *inst )
{
  setInstance( inst, true );
}

void PartBase::setInstance( KInstance *inst, bool bLoadPlugins )
{
  KXMLGUIClient::setInstance( inst );
  KGlobal::locale()->insertCatalog( inst->instanceName() );
  // install 'instancename'data resource type
  KGlobal::dirs()->addResourceType( inst->instanceName() + "data",
                                    KStandardDirs::kde_default( "data" )
                                    + QString::fromLatin1( inst->instanceName() ) + '/' );
  if ( bLoadPlugins )
    loadPlugins( m_obj, this, instance() );
}

void PartBase::loadPlugins( QObject *parent, KXMLGUIClient *parentGUIClient, KInstance *instance )
{
  if( d->m_pluginLoadingMode != DoNotLoadPlugins )
    Plugin::loadPlugins( parent, parentGUIClient, instance, d->m_pluginLoadingMode == LoadPlugins );
}

void PartBase::setPluginLoadingMode( PluginLoadingMode loadingMode )
{
    d->m_pluginLoadingMode = loadingMode;
}

Part::Part( QObject *parent )
 : QObject( parent )
{
  d = new PartPrivate;
  m_widget = 0L;
  m_manager = 0L;
  PartBase::setPartObject( this );
}

Part::~Part()
{
  kdDebug(1000) << "Part::~Part " << this << endl;

  if ( m_widget )
  {
    // We need to disconnect first, to avoid calling it !
    disconnect( m_widget, SIGNAL( destroyed() ),
                this, SLOT( slotWidgetDestroyed() ) );
  }

  if ( m_manager )
    m_manager->removePart(this);

  if ( m_widget )
  {
    kdDebug(1000) << "deleting widget " << m_widget << " " << m_widget->name() << endl;
    delete (QWidget*) m_widget;
  }

  delete d;
}

void Part::embed( QWidget * parentWidget )
{
  if ( widget() )
    widget()->reparent( parentWidget, 0, QPoint( 0, 0 ), true );
}

QWidget *Part::widget()
{
  return m_widget;
}

void Part::setManager( PartManager *manager )
{
  m_manager = manager;
}

PartManager *Part::manager() const
{
  return m_manager;
}

Part *Part::hitTest( QWidget *widget, const QPoint & )
{
  if ( (QWidget *)m_widget != widget )
    return 0L;

  return this;
}

void Part::setWidget( QWidget *widget )
{
  assert ( !m_widget ); // otherwise we get two connects
  m_widget = widget;
  connect( m_widget, SIGNAL( destroyed() ),
           this, SLOT( slotWidgetDestroyed() ) );

  // Tell the actionCollection() which widget its
  //  action shortcuts should be connected to.
  actionCollection()->setWidget( widget );

  // Since KParts objects are XML-based, shortcuts should
  //  be connected to the widget when the XML settings
  //  are processed, rather than on KAction construction.
  actionCollection()->setAutoConnectShortcuts( false );
}

void Part::setSelectable( bool selectable )
{
  d->m_bSelectable = selectable;
}

bool Part::isSelectable() const
{
  return d->m_bSelectable;
}

void Part::customEvent( QEvent *event )
{
  if ( PartActivateEvent::test( event ) )
  {
    partActivateEvent( (PartActivateEvent *)event );
    return;
  }

  if ( PartSelectEvent::test( event ) )
  {
    partSelectEvent( (PartSelectEvent *)event );
    return;
  }

  if ( GUIActivateEvent::test( event ) )
  {
    guiActivateEvent( (GUIActivateEvent *)event );
    return;
  }

  QObject::customEvent( event );
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
    return 0L;

  return factory()->container( containerName, this );
}

void Part::slotWidgetDestroyed()
{
  kdDebug(1000) << "KPart::slotWidgetDestroyed(), deleting part " << name() << endl;
  m_widget = 0;
  delete this;
}

//////////////////////////////////////////////////

namespace KParts
{

class ReadOnlyPartPrivate
{
public:
  ReadOnlyPartPrivate()
  {
    m_job = 0L;
    m_uploadJob = 0L;
    m_showProgressInfo = true;
    m_saveOk = false;
    m_waitForSave = false;
    m_duringSaveAs = false;
  }
  ~ReadOnlyPartPrivate()
  {
  }

  KIO::FileCopyJob * m_job;
  KIO::FileCopyJob * m_uploadJob;
  KURL m_originalURL;
  bool m_showProgressInfo : 1;
  bool m_saveOk : 1;
  bool m_waitForSave : 1;
  bool m_duringSaveAs : 1;
};

}

ReadOnlyPart::ReadOnlyPart( QObject *parent )
 : Part( parent ), m_bTemp( false )
{
  d = new ReadOnlyPartPrivate;
}

ReadOnlyPart::~ReadOnlyPart()
{
  ReadOnlyPart::closeURL();
  delete d;
}

void ReadOnlyPart::setProgressInfoEnabled( bool show )
{
  d->m_showProgressInfo = show;
}

bool ReadOnlyPart::isProgressInfoEnabled() const
{
  return d->m_showProgressInfo;
}

#ifndef KDE_NO_COMPAT
void ReadOnlyPart::showProgressInfo( bool show )
{
  d->m_showProgressInfo = show;
}
#endif

bool ReadOnlyPart::openURL( const KURL &url )
{
  if ( !url.isValid() )
    return false;
  if ( !closeURL() )
    return false;
  m_url = url;
  if ( m_url.isLocalFile() )
  {
    emit started( 0 );
    m_file = m_url.path();
    bool ret = openFile();
    if (ret)
    {
        emit completed();
        emit setWindowCaption( m_url.prettyURL() );
    };
    return ret;
  }
  else
  {
    m_bTemp = true;
    // Use same extension as remote file. This is important for mimetype-determination (e.g. koffice)
    QString fileName = url.fileName();
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.extension();
    QString extension;
    if ( !ext.isEmpty() && url.query().isNull() ) // not if the URL has a query, e.g. cgi.pl?something
        extension = "."+ext; // keep the '.'
    KTempFile tempFile( QString::null, extension );
    m_file = tempFile.name();

    KURL destURL;
    destURL.setPath( m_file );
    d->m_job = KIO::file_copy( m_url, destURL, 0600, true, false, d->m_showProgressInfo );
    d->m_job->setWindow( widget() ? widget()->topLevelWidget() : 0 );
    emit started( d->m_job );
    connect( d->m_job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotJobFinished ( KIO::Job * ) ) );
    return true;
  }
}

void ReadOnlyPart::abortLoad()
{
  if ( d->m_job )
  {
    //kdDebug(1000) << "Aborting job " << d->m_job << endl;
    d->m_job->kill();
    d->m_job = 0;
  }
}

bool ReadOnlyPart::closeURL()
{
  abortLoad(); //just in case

  if ( m_bTemp )
  {
    unlink( QFile::encodeName(m_file) );
    m_bTemp = false;
  }
  // It always succeeds for a read-only part,
  // but the return value exists for reimplementations
  // (e.g. pressing cancel for a modified read-write part)
  return true;
}

void ReadOnlyPart::slotJobFinished( KIO::Job * job )
{
  kdDebug(1000) << "ReadOnlyPart::slotJobFinished" << endl;
  assert( job == d->m_job );
  d->m_job = 0;
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    if ( openFile() )
      emit setWindowCaption( m_url.prettyURL() );
    emit completed();
  }
}

void ReadOnlyPart::guiActivateEvent( GUIActivateEvent * event )
{
  if (event->activated())
  {
    if (!m_url.isEmpty())
    {
      kdDebug(1000) << "ReadOnlyPart::guiActivateEvent -> " << m_url.prettyURL() << endl;
      emit setWindowCaption( m_url.prettyURL() );
    } else emit setWindowCaption( "" );
  }
}

bool ReadOnlyPart::openStream( const QString& mimeType, const KURL& url )
{
  if ( !closeURL() )
    return false;
  m_url = url;
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

//////////////////////////////////////////////////

ReadWritePart::ReadWritePart( QObject *parent )
 : ReadOnlyPart( parent ), m_bModified( false ), m_bClosing( false )
{
  m_bReadWrite = true;
}

ReadWritePart::~ReadWritePart()
{
  // parent destructor will delete temp file
  // we can't call our own closeURL() here, because
  // "cancel" wouldn't cancel anything. We have to assume
  // the app called closeURL() before destroying us.
}

void ReadWritePart::setReadWrite( bool readwrite )
{
  // Perhaps we should check isModified here and issue a warning if true
  m_bReadWrite = readwrite;
}

void ReadWritePart::setModified( bool modified )
{
  kdDebug(1000) << "ReadWritePart::setModified( " << (modified ? "true" : "false") << ")" << endl;
  if ( !m_bReadWrite && modified )
  {
      kdError(1000) << "Can't set a read-only document to 'modified' !" << endl;
      return;
  }
  m_bModified = modified;
}

void ReadWritePart::setModified()
{
  setModified( true );
}

bool ReadWritePart::queryClose()
{
  if ( !isReadWrite() || !isModified() )
    return true;

  QString docName = url().fileName();
  if (docName.isEmpty()) docName = i18n( "Untitled" );

  int res = KMessageBox::warningYesNoCancel( widget(),
          i18n( "The document \"%1\" has been modified.\n"
                "Do you want to save your changes or discard them?" ).arg( docName ),
          i18n( "Close Document" ), KStdGuiItem::save(), KStdGuiItem::discard() );

  bool abortClose=false;
  bool handled=false;

  switch(res) {
  case KMessageBox::Yes :
    sigQueryClose(&handled,&abortClose);
    if (!handled)
    {
      if (m_url.isEmpty())
      {
          KURL url = KFileDialog::getSaveURL();
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

bool ReadWritePart::closeURL()
{
  abortLoad(); //just in case
  if ( isReadWrite() && isModified() )
  {
    if (!queryClose())
       return false;
  }
  // Not modified => ok and delete temp file.
  return ReadOnlyPart::closeURL();
}

bool ReadWritePart::closeURL( bool promptToSave )
{
  return promptToSave ? closeURL() : ReadOnlyPart::closeURL();
}

bool ReadWritePart::save()
{
  d->m_saveOk = false;
  if ( m_file.isEmpty() ) // document was created empty
      prepareSaving();
  if( saveFile() )
    return saveToURL();
  else
    emit canceled(QString::null);
  return false;
}

bool ReadWritePart::saveAs( const KURL & kurl )
{
  if (!kurl.isValid())
  {
      kdError(1000) << "saveAs: Malformed URL " << kurl.url() << endl;
      return false;
  }
  d->m_duringSaveAs = true;
  d->m_originalURL = m_url;
  m_url = kurl; // Store where to upload in saveToURL
  prepareSaving();
  bool result = save(); // Save local file and upload local file
  if (result)
    emit setWindowCaption( m_url.prettyURL() );
  else
  {
    m_url = d->m_originalURL;
    d->m_duringSaveAs = false;
    d->m_originalURL = KURL();
  }

  return result;
}

// Set m_file correctly for m_url
void ReadWritePart::prepareSaving()
{
  // Local file
  if ( m_url.isLocalFile() )
  {
    if ( m_bTemp ) // get rid of a possible temp file first
    {              // (happens if previous url was remote)
      unlink( QFile::encodeName(m_file) );
      m_bTemp = false;
    }
    m_file = m_url.path();
  }
  else
  { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( m_file.isEmpty() || !m_bTemp )
    {
      KTempFile tempFile;
      m_file = tempFile.name();
      m_bTemp = true;
    }
    // otherwise, we already had a temp file
  }
}

bool ReadWritePart::saveToURL()
{
  if ( m_url.isLocalFile() )
  {
    setModified( false );
    emit completed();
    // if m_url is a local file there won't be a temp file -> nothing to remove
    assert( !m_bTemp );
    d->m_saveOk = true;
    d->m_duringSaveAs = false;
    d->m_originalURL = KURL();
    return true; // Nothing to do
  }
  else
  {
    if (d->m_uploadJob)
    {
       unlink(QFile::encodeName(d->m_uploadJob->srcURL().path()));
       d->m_uploadJob->kill();
       d->m_uploadJob = 0;
    }
    KTempFile tempFile;
    QString uploadFile = tempFile.name();
    KURL uploadUrl;
    uploadUrl.setPath( uploadFile );
    tempFile.unlink();
    // Create hardlink
    if (::link(QFile::encodeName(m_file), QFile::encodeName(uploadFile)) != 0)
    {
       // Uh oh, some error happened.
       return false;
    }
    d->m_uploadJob = KIO::file_move( uploadUrl, m_url, -1, true /*overwrite*/ );
    d->m_uploadJob->setWindow( widget() ? widget()->topLevelWidget() : 0 );
    connect( d->m_uploadJob, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotUploadFinished (KIO::Job *) ) );
    return true;
  }
}

void ReadWritePart::slotUploadFinished( KIO::Job * )
{
  if (d->m_uploadJob->error())
  {
    unlink(QFile::encodeName(d->m_uploadJob->srcURL().path()));
    QString error = d->m_uploadJob->errorString();
    d->m_uploadJob = 0;
    if (d->m_duringSaveAs)
      m_url = d->m_originalURL;
    emit canceled( error );
  }
  else
  {
    KDirNotify_stub allDirNotify("*", "KDirNotify*");
    KURL dirUrl( m_url );
    dirUrl.setPath( dirUrl.directory() );
    allDirNotify.FilesAdded( dirUrl );

    d->m_uploadJob = 0;
    setModified( false );
    emit completed();
    d->m_saveOk = true;
  }
  d->m_duringSaveAs = false;
  d->m_originalURL = KURL();
  if (d->m_waitForSave)
  {
    emit leaveModality();
  }
}

bool ReadWritePart::waitSaveComplete()
{
  if (!d->m_uploadJob)
     return d->m_saveOk;

  d->m_waitForSave = true;

  QEventLoop eventLoop;
  connect(this, SIGNAL(leaveModality()),
          &eventLoop, SLOT(quit()));
  eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

  d->m_waitForSave = false;

  return d->m_saveOk;
}

#include "part.moc"

// vim:sw=2:ts=8:et

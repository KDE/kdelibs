#include <kparts/part.h>
#include <kparts/event.h>
#include <kparts/plugin.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>

#include <qfile.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtextstream.h>

#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kstddirs.h>

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <kdebug.h>

using namespace KParts;

namespace KParts
{

template class QList<KXMLGUIServant>;

class PartBasePrivate
{
public:
  PartBasePrivate()
  {
  }
  ~PartBasePrivate()
  {
  }
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
};

PartBase::PartBase()
{
  d = new PartBasePrivate;
  m_obj = 0L;
}

PartBase::~PartBase()
{
  delete d;
}

void PartBase::setObject( QObject *obj )
{
  m_obj = obj;
}

void PartBase::setInstance( KInstance *inst )
{
  setInstance( inst, true );
}

void PartBase::setInstance( KInstance *inst, bool loadPlugins )
{
  KXMLGUIBase::setInstance( inst );
  if ( loadPlugins )
    Plugin::loadPlugins( m_obj, instance() );
}

Part::Part( QObject *parent, const char* name )
 : QObject( parent, name )
{
  d = new PartPrivate;
  m_widget = 0L;
  m_manager = 0L;
  PartBase::setObject( this );
}

Part::~Part()
{
  if ( m_widget )
  {
    // We need to disconnect first, to avoid calling it !
    disconnect( m_widget, SIGNAL( destroyed() ),
                this, SLOT( slotWidgetDestroyed() ) );
    kDebugInfo( 1000, QString("***** deleting widget '%1'").arg(m_widget->name()) );
    delete (QWidget *)m_widget;
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

PartManager *Part::manager()
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
}

void Part::setSelectable( bool selectable )
{
  d->m_bSelectable = selectable;
}

bool Part::isSelectable() const
{
  return d->m_bSelectable;
}

bool Part::event( QEvent *event )
{
  if ( QObject::event( event ) )
    return true;

  if ( PartActivateEvent::test( event ) )
  {
    partActivateEvent( (PartActivateEvent *)event );
    return true;
  }

  if ( PartSelectEvent::test( event ) )
  {
    partSelectEvent( (PartSelectEvent *)event );
    return true;
  }

  if ( GUIActivateEvent::test( event ) )
  {
    guiActivateEvent( (GUIActivateEvent *)event );
    return true;
  }

  return false;
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
  kDebugInfo( 1000, QString(" ********** KPart::slotWidgetDestroyed(), deleting part '%1'").arg(name()) );
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
  }
  ~ReadOnlyPartPrivate()
  {
  }

  KIO::FileCopyJob * m_job;
};

};

ReadOnlyPart::ReadOnlyPart( QObject *parent, const char *name )
 : Part( parent, name ), m_bTemp( false )
{
  d = new ReadOnlyPartPrivate;
}

ReadOnlyPart::~ReadOnlyPart()
{
  closeURL();
  delete d;
}

bool ReadOnlyPart::openURL( const KURL &url )
{
  if ( url.isMalformed() )
    return false;
  if ( !closeURL() )
    return false;
  m_url = url;
  emit setWindowCaption( m_url.decodedURL() );
  if ( m_url.isLocalFile() )
  {
    emit started( 0 );
    m_file = m_url.path();
    bool ret = openFile();
    emit completed();
    return ret;
  }
  else
  {
    m_bTemp = true;
    m_file = tmpnam(0);
    // We can't use mkstemp since we don't want to create the file here
    // KIO::Job has to create it

    d->m_job = KIO::file_copy( m_url, m_file );
    emit started( d->m_job );
    connect( d->m_job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotJobFinished ( KIO::Job * ) ) );
    return true;
  }
}

void ReadOnlyPart::abortLoad()
{
  if ( d->m_job )
  {
    d->m_job->kill();
    d->m_job = 0;
  }
}

bool ReadOnlyPart::closeURL()
{
  abortLoad(); //just in case

  if ( m_bTemp )
  {
    unlink( m_file.ascii() );
    m_bTemp = false;
  }
  // It always succeeds for a read-only part,
  // but the return value exists for reimplementations
  // (e.g. pressing cancel for a modified read-write part)
  return true;
}

void ReadOnlyPart::slotJobFinished( KIO::Job * job )
{
  assert( job == d->m_job );
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    openFile();
    emit completed();
  }
  d->m_job = 0;
}

void ReadOnlyPart::guiActivateEvent( GUIActivateEvent * event )
{
  if (event->activated())
  {
    if (!m_url.isEmpty())
    {
      kDebugInfo( 1000, "ReadOnlyPart::guiActivateEvent -> %s", m_url.decodedURL().ascii() );
      emit setWindowCaption( m_url.decodedURL() );
    } else emit setWindowCaption( "" );
  }
}

//////////////////////////////////////////////////

ReadWritePart::ReadWritePart( QObject *parent, const char *name )
 : ReadOnlyPart( parent, name ), m_bModified( false ), m_bClosing( false )
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
  m_bModified = modified;
}

void ReadWritePart::setModified()
{
  if ( !m_bReadWrite )
  {
      kDebugError( 1000, "Can't set a read-only document to 'modified' !" );
      return;
  }
  setModified( true );
}

bool ReadWritePart::closeURL()
{
  abortLoad(); //just in case
  if ( m_bModified && m_bReadWrite && !m_url.isEmpty() )
  {
    int res = KMessageBox::warningYesNoCancel( 0L,
            i18n( "The document has been modified\nDo you want to save it ?" ));

    switch(res) {
    case KMessageBox::Yes :
      m_bClosing = true; // remember to clean up the temp file
      return save();
    case KMessageBox::No :
      return true;
    default : // case KMessageBox::Cancel :
      return false;
    }
  }
  // Not modified => ok and delete temp file.
  return ReadOnlyPart::closeURL();
}

bool ReadWritePart::save()
{
  return saveFile() && saveToURL();
}

bool ReadWritePart::saveAs( const KURL & kurl )
{
  if (kurl.isMalformed())
      return false;
  m_url = kurl; // Store where to upload in saveToURL
  // Local file
  if ( m_url.isLocalFile() )
  {
    if ( m_bTemp ) // get rid of a possible temp file first
    {              // (happens if previous url was remote)
      unlink( m_file.ascii() );
      m_bTemp = false;
    }
    m_file = m_url.path();
  }
  else
  { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( m_file.isEmpty() || !m_bTemp )
    {
      m_file = tmpnam(0);
      m_bTemp = true;
    }
    // otherwise, we already had a temp file
  }
  return save(); // Save local file and upload local file
}

bool ReadWritePart::saveToURL()
{
  if ( m_url.isLocalFile() )
  {
    m_bModified = false;
    emit completed();
    return true; // Nothing to do
  }
  else
  {
    KIO::Job * job = KIO::file_copy( m_file, m_url );
    connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotUploadFinished (KIO::Job *) ) );
    return true;
  }
}

void ReadWritePart::slotUploadFinished( KIO::Job * job )
{
  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    m_bModified = false;
    if ( m_bClosing )
      ReadOnlyPart::closeURL();
    emit completed();
  }
}

#include "part.moc"

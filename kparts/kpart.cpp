#include "kpart.h"
#include "kplugin.h"

#include <qfile.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpainter.h>
#include <qtextstream.h>

#include <kinstance.h>
#include <klibloader.h>
#include <kio_job.h>
#include <kstddirs.h>

#include <stdio.h>
#include <unistd.h>

KPart::KPart( const char* name )
    : QObject( 0L, name ), m_collection( this )
{
  m_widget = 0;
  m_host = 0;
  m_bPluginActionsMerged = false;
}

KPart::~KPart()
{
  if ( m_widget )
  {
    // We need to disconnect first, to avoid calling it !
    disconnect( m_widget, SIGNAL( destroyed() ),
                this, SLOT( slotWidgetDestroyed() ) );
    qDebug(QString("***** deleting widget '%1'").arg(m_widget->name()));
    delete (QWidget *)m_widget;
  }
}

void KPart::embed( QWidget * parentWidget )
{
  m_widget->reparent( parentWidget, 0, QPoint( 0, 0 ), true );
}

QStringList KPart::pluginActionDocuments()
{
  if ( !instance() )
    return QStringList();
  
#warning FIX THIS WHEN MOVING TO KDELIBS
  return instance()->dirs()->findAllResources( "appdata", "*", true, false );
}

QDomDocument KPart::mergedActionDOM()
{
  if ( m_bPluginActionsMerged )
    return m_mergedDOM;

 QStringList pluginDocuments = pluginActionDocuments();
 if ( pluginDocuments.count() == 0 && instance() )
  {
    qDebug( "no plugins found for %s", instance()->instanceName().data() );
    m_mergedDOM.setContent( config() );
    m_bPluginActionsMerged = true;
    return m_mergedDOM;
  }

  QStringList::ConstIterator pluginIt = pluginDocuments.begin();
  QStringList::ConstIterator pluginEnd = pluginDocuments.end();

  QDomDocument pluginDoc;
  
  for (; pluginIt !=  pluginEnd; ++pluginIt )
  {
    QString xml = KXMLGUIFactory::readConfigFile( *pluginIt );
  
    if ( pluginDoc.documentElement().isNull() )
      pluginDoc.setContent( xml );
    else
    {
      QDomDocument tempDoc;
      tempDoc.setContent( xml );
      QDomElement docElement = tempDoc.documentElement();
      if ( !docElement.isNull() )
        KXMLGUIFactory::mergeXML( pluginDoc.documentElement(), docElement );
    }
  }

  m_mergedDOM.setContent( config() );
  
  KXMLGUIFactory::mergeXML( m_mergedDOM.documentElement(), pluginDoc.documentElement() );

  m_bPluginActionsMerged = true;

  return m_mergedDOM;
} 

void KPart::setWidget( QWidget *widget )
{
  m_widget = widget;
  connect( m_widget, SIGNAL( destroyed() ),
	   this, SLOT( slotWidgetDestroyed() ) );
}

QString KPart::readConfigFile( const QString& filename ) const
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return QString::null;

    uint size = file.size();
    char* buffer = new char[ size + 1 ];
    file.readBlock( buffer, size );
    buffer[ size ] = 0;
    file.close();

    QString text = QString::fromUtf8( buffer, size );
    delete[] buffer;

    return text;
}

QString KPart::config()
{
    if (  m_config.isEmpty() )
	m_config = configFile();

    return m_config;
}

QAction* KPart::action( const char* name )
{
    return m_collection.action( name );
}

QActionCollection* KPart::actionCollection()
{
    return &m_collection;
}

KPlugin* KPart::plugin( const char* libname )
{
    QObject* ch = child( libname, "KPlugin" );
    if ( ch )
	return (KPlugin*)ch;

    KLibLoader* loader = KLibLoader::self();
    if ( !loader )
    {
	qDebug("KPart: No library loader installed");
	return 0;
    }

    KLibFactory* f = loader->factory( libname );
    if ( !f )
    {
	qDebug("KPart: Could not initialize library");
	return 0;
    }
    QObject* obj = f->create( this, libname, "KPlugin" );
    if ( !obj->inherits("KPlugin" ) )
    {
	qDebug("The library does not feature an object of class Plugin");
	delete obj;
	return 0;
    }

    return (KPlugin*)obj;
}

void KPart::slotWidgetDestroyed()
{
  debug(QString(" ********** KPart::slotWidgetDestroyed(), deleting part '%1'").arg(name()));
  m_widget = 0;
  delete this;
}

//////////////////////////////////////////////////

KReadOnlyPart::KReadOnlyPart( const char *name )
 : KPart( name ), m_bTemp( false )
{
}

KReadOnlyPart::~KReadOnlyPart()
{
  closeURL();
}

void KReadOnlyPart::init()
{
}

bool KReadOnlyPart::openURL( const KURL &url )
{
  if ( url.isMalformed() )
    return false;
  emit started();
  closeURL();
  m_url = url;
  if ( m_url.isLocalFile() )
  {
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
    // KIOJob has to create it

    KIOJob * job = new KIOJob;
    connect( job, SIGNAL( sigFinished (int) ), this, SLOT( slotJobFinished (int) ) );
    connect( job, SIGNAL( sigError( int, int, const char * ) ),
             this, SLOT( slotJobError ( int, int, const char * ) ) );
    job->copy( m_url.url(), m_file );
    return true;
  }
}

void KReadOnlyPart::closeURL()
{
  if ( m_bTemp )
  {
    unlink( m_file.ascii() );
    m_bTemp = false;
  }
}

void KReadOnlyPart::slotJobFinished( int /*_id*/ )
{
  openFile();
  emit completed();
}

void KReadOnlyPart::slotJobError( int, int, const char * text )
{
  emit canceled( QString(text) );
}

//////////////////////////////////////////////////

KReadWritePart::KReadWritePart( const char *name )
 : KReadOnlyPart( name ), m_bModified( false )
{
}

KReadWritePart::~KReadWritePart()
{
  // parent destructor will delete temp file
}

bool KReadWritePart::saveAs( const KURL & kurl )
{
  if (kurl.isMalformed())
      return false;
  m_url = kurl; // Store where to upload in saveToURL
  return save() && saveToURL(); // Save local file and upload local file
}

bool KReadWritePart::saveToURL()
{
  if ( m_url.isLocalFile() )
  {
    m_bModified = false;
    return true; // Nothing to do
  }
  else
  {
    KIOJob * job = new KIOJob;
    connect( job, SIGNAL( sigFinished (int) ), this, SLOT( slotUploadFinished (int) ) );
    connect( job, SIGNAL( sigError( int, int, const char * ) ),
             this, SLOT( slotUploadError ( int, int, const char * ) ) );
    job->copy( m_file, m_url.url() );
    return true;
  }
}

void KReadWritePart::slotUploadFinished( int /*_id*/ )
{
  m_bModified = false;
  emit completed();
}

void KReadWritePart::slotUploadError( int, int, const char * text )
{
  emit canceled( QString(text) );
}

//////////////////////////////////////////////////

KPartGUIServant::KPartGUIServant( KPart *part )
  : QObject( part )
{
  m_part = part;
}

QAction *KPartGUIServant::action( const QDomElement &element )
{
  QString pluginAttr = element.attribute( "plugin" );
  QString name = element.attribute( "name" );

  if ( !pluginAttr.isEmpty() )
  {
    KPlugin *plugin = m_part->plugin( pluginAttr );

    if ( !plugin )
      return 0;

    return plugin->action( name );
  }

  return m_part->action( name );
}

QDomDocument KPartGUIServant::document()
{
  return m_part->mergedActionDOM();
}

#include "kpart.moc"

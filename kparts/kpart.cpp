#include "kpart.h"

#include <qfile.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpainter.h>

#include <klibloader.h>
#include <kio_job.h>

#include <stdio.h>
#include <unistd.h>

KPart::KPart( QWidget* parent, const char* name )
    : QWidget( parent, name ), m_collection( this )
{
}

KPart::~KPart()
{
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

//////////////////////////////////////////////////

KReadOnlyPart::KReadOnlyPart( QWidget *parent, const char *name )
 : KPart( parent, name ), m_bTemp( false )
{
}

KReadOnlyPart::~KReadOnlyPart()
{
  closeURL();
}

void KReadOnlyPart::init()
{
}

bool KReadOnlyPart::openURL( const QString &url )
{
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
    job->copy( url, m_file );
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

KReadWritePart::KReadWritePart( QWidget *parent, const char *name )
 : KReadOnlyPart( parent, name )
{
}

KReadWritePart::~KReadWritePart()
{
}

// TODO closeURL : if ( isModified && m_bTemp) => uploads (asking confirmation first)

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

QString KPartGUIServant::xml()
{
  return m_part->config();
}

#include "kpart.moc"

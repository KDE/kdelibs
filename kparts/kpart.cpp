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
#include <assert.h>
#include <kdebug.h>

using namespace KParts;

namespace KParts
{

template class QList<XMLGUIServant>;

class PartPrivate
{
public:
  PartPrivate()
  {
    m_bPluginsDirty = true;
    m_bDocDirty = true;
    m_pluginServants.setAutoDelete( true );
    m_servant = 0L;
  }
  ~PartPrivate()
  {
    if ( m_servant )
      delete m_servant;
  }

  bool m_bPluginsDirty;
  bool m_bDocDirty;
  QValueList<QDomDocument> m_plugins;
  QList<XMLGUIServant> m_pluginServants;
  QDomDocument m_doc;
  XMLGUIServant *m_servant;
};
};

Part::Part( const char* name )
    : QObject( 0L, name ), m_collection( this )
{
  d = new PartPrivate;
  m_widget = 0;
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
  m_widget->reparent( parentWidget, 0, QPoint( 0, 0 ), true );
}

void Part::updatePlugins()
{
  d->m_bPluginsDirty = true;
}

QStringList Part::plugins()
{
  if ( !instance() )
    return QStringList();

  return instance()->dirs()->findAllResources( "data", instance()->instanceName()+"/kpartplugins/*", true, false );
}

QValueList<QDomDocument> Part::pluginDocuments()
{
  if ( d->m_bPluginsDirty )
  {
    d->m_plugins.clear();

    QStringList pluginDocs = plugins();
    QStringList::ConstIterator pIt = pluginDocs.begin();
    QStringList::ConstIterator pEnd = pluginDocs.end();
    for (; pIt != pEnd; ++pIt )
    {
      QString xml = XMLGUIFactory::readConfigFile( *pIt );
      if ( !xml.isEmpty() )
      {
        QDomDocument doc;
        doc.setContent( xml );
        if ( !doc.documentElement().isNull() )
          d->m_plugins.append( doc );
      }
    }

  }

  return d->m_plugins;
}

const QList<XMLGUIServant> *Part::pluginServants()
{
  if ( d->m_bPluginsDirty )
  {
    d->m_pluginServants.clear();

    QValueList<QDomDocument> pluginDocs = pluginDocuments();
    QValueList<QDomDocument>::ConstIterator pIt = pluginDocs.begin();
    QValueList<QDomDocument>::ConstIterator pEnd = pluginDocs.end();
    for (; pIt != pEnd; ++pIt )
    {
      PartGUIServant *pluginServant = new PartGUIServant( this, *pIt );
      d->m_pluginServants.append( pluginServant );
    }

    d->m_bPluginsDirty = false;
  }

  return &d->m_pluginServants;
}

QDomDocument Part::document() const
{
  return d->m_doc;
}

XMLGUIServant *Part::servant()
{
  if ( d->m_bDocDirty )
  {
    if ( d->m_servant )
      delete d->m_servant;

    d->m_servant = new PartGUIServant( this, document() );
    d->m_bDocDirty = false;
  }

  return d->m_servant;
}

void Part::setWidget( QWidget *widget )
{
  assert ( !m_widget ); // otherwise we get two connects
  m_widget = widget;
  connect( m_widget, SIGNAL( destroyed() ),
	   this, SLOT( slotWidgetDestroyed() ) );
}

void Part::setXMLFile( QString file )
{
  if ( file[0] != '/' )
    file = locate( "data", QString(instance()->instanceName())+"/"+file );

  QString xml = XMLGUIFactory::readConfigFile( file );
  setXML( xml );
}

void Part::setXML( const QString &document )
{
  d->m_doc.setContent( document );
  d->m_bDocDirty = true;
}

QAction* Part::action( const char* name )
{
    return m_collection.action( name );
}

QActionCollection* Part::actionCollection()
{
    return &m_collection;
}

Plugin* Part::plugin( const char* libname )
{
    QObject* ch = child( libname, "KPlugin" );
    if ( ch )
	return (Plugin*)ch;

    KLibLoader* loader = KLibLoader::self();
    if ( !loader )
    {
	kDebugError( 1000, "KPart: No library loader installed" );
	return 0;
    }

    KLibFactory* f = loader->factory( libname );
    if ( !f )
    {
	kDebugError( 1000, "KPart: Could not initialize library" );
	return 0;
    }
    QObject* obj = f->create( this, libname, "KPlugin" );
    if ( !obj->inherits("KPlugin" ) )
    {
	kDebugError( 1000, "The library does not feature an object of class Plugin" );
	delete obj;
	return 0;
    }

    return (Plugin*)obj;
}

QAction *Part::action( const QDomElement &element )
{
  QString pluginAttr = element.attribute( "plugin" );
  QString name = element.attribute( "name" );

  if ( !pluginAttr.isEmpty() )
  {
    Plugin *thePlugin = plugin( pluginAttr );

    if ( !thePlugin )
      return 0;

    return thePlugin->action( name );
  }

  return action( name.ascii() );
}

void Part::slotWidgetDestroyed()
{
  kDebugInfo( 1000, QString(" ********** KPart::slotWidgetDestroyed(), deleting part '%1'").arg(name()) );
  m_widget = 0;
  delete this;
}

//////////////////////////////////////////////////

ReadOnlyPart::ReadOnlyPart( const char *name )
 : Part( name ), m_bTemp( false )
{
}

ReadOnlyPart::~ReadOnlyPart()
{
  closeURL();
}

void ReadOnlyPart::init()
{
}

bool ReadOnlyPart::openURL( const KURL &url )
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

void ReadOnlyPart::closeURL()
{
  if ( m_bTemp )
  {
    unlink( m_file.ascii() );
    m_bTemp = false;
  }
}

void ReadOnlyPart::slotJobFinished( int /*_id*/ )
{
  openFile();
  emit completed();
}

void ReadOnlyPart::slotJobError( int, int, const char * text )
{
  emit canceled( QString(text) );
}

//////////////////////////////////////////////////

ReadWritePart::ReadWritePart( const char *name )
 : ReadOnlyPart( name ), m_bModified( false )
{
}

ReadWritePart::~ReadWritePart()
{
  // parent destructor will delete temp file
}

bool ReadWritePart::saveAs( const KURL & kurl )
{
  if (kurl.isMalformed())
      return false;
  m_url = kurl; // Store where to upload in saveToURL
  return save() && saveToURL(); // Save local file and upload local file
}

bool ReadWritePart::saveToURL()
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

void ReadWritePart::slotUploadFinished( int /*_id*/ )
{
  m_bModified = false;
  emit completed();
}

void ReadWritePart::slotUploadError( int, int, const char * text )
{
  emit canceled( QString(text) );
}

//////////////////////////////////////////////////

PartGUIServant::PartGUIServant( Part *part, const QDomDocument &document )
  : QObject( part )
{
  m_part = part;
  m_doc = document;
}

QAction *PartGUIServant::action( const QDomElement &element )
{
  return m_part->action( element );
}

QDomDocument PartGUIServant::document()
{
  return m_doc;
}

#include "kpart.moc"

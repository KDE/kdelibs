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

class KPartPrivate
{
public:
  KPartPrivate()
  {
    m_bPluginsDirty = true;
    m_bDocDirty = true;
    m_pluginServants.setAutoDelete( true );
    m_servant = 0L;
  }
  ~KPartPrivate()
  {
    if ( m_servant )
      delete m_servant;
  }

  bool m_bPluginsDirty;
  bool m_bDocDirty;
  QValueList<QDomDocument> m_plugins;
  QList<KXMLGUIServant> m_pluginServants;
  QDomDocument m_doc;
  KXMLGUIServant *m_servant;
};

KPart::KPart( const char* name )
    : QObject( 0L, name ), m_collection( this )
{
  d = new KPartPrivate;
  m_widget = 0;
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

  delete d;
}

void KPart::embed( QWidget * parentWidget )
{
  m_widget->reparent( parentWidget, 0, QPoint( 0, 0 ), true );
}

void KPart::updatePlugins()
{
  d->m_bPluginsDirty = true;
}

QStringList KPart::plugins()
{
  if ( !instance() )
    return QStringList();

#warning FIX THIS WHEN MOVING TO KDELIBS
  return instance()->dirs()->findAllResources( "appdata", "*", true, false );
}

QValueList<QDomDocument> KPart::pluginDocuments()
{
  if ( d->m_bPluginsDirty )
  {
    d->m_plugins.clear();

    QStringList pluginDocs = plugins();
    QStringList::ConstIterator pIt = pluginDocs.begin();
    QStringList::ConstIterator pEnd = pluginDocs.end();
    for (; pIt != pEnd; ++pIt )
    {
      QString xml = KXMLGUIFactory::readConfigFile( *pIt );
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

const QList<KXMLGUIServant> *KPart::pluginServants()
{
  if ( d->m_bPluginsDirty )
  {
    d->m_pluginServants.clear();

    QValueList<QDomDocument> pluginDocs = pluginDocuments();
    QValueList<QDomDocument>::ConstIterator pIt = pluginDocs.begin();
    QValueList<QDomDocument>::ConstIterator pEnd = pluginDocs.end();
    for (; pIt != pEnd; ++pIt )
    {
      KPartGUIServant *pluginServant = new KPartGUIServant( this, *pIt );
      d->m_pluginServants.append( pluginServant );
    }

    d->m_bPluginsDirty = false;
  }

  return &d->m_pluginServants;
}

QDomDocument KPart::document() const
{
  return d->m_doc;
}

KXMLGUIServant *KPart::servant()
{
  if ( d->m_bDocDirty )
  {
    if ( d->m_servant )
      delete d->m_servant;

    d->m_servant = new KPartGUIServant( this, document() );
    d->m_bDocDirty = false;
  }

  return d->m_servant;
}

void KPart::setWidget( QWidget *widget )
{
  assert ( !m_widget ); // otherwise we get two connects
  m_widget = widget;
  connect( m_widget, SIGNAL( destroyed() ),
	   this, SLOT( slotWidgetDestroyed() ) );
}

void KPart::setXMLFile( const QString & file )
{
  QString xml = KXMLGUIFactory::readConfigFile( file );
  setXML( xml );
}

void KPart::setXML( const QString &document )
{
  d->m_doc.setContent( document );
  d->m_bDocDirty = true;
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

QAction *KPart::action( const QDomElement &element )
{
  QString pluginAttr = element.attribute( "plugin" );
  QString name = element.attribute( "name" );

  if ( !pluginAttr.isEmpty() )
  {
    KPlugin *thePlugin = plugin( pluginAttr );

    if ( !thePlugin )
      return 0;

    return thePlugin->action( name );
  }

  return action( name.ascii() );
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

KPartGUIServant::KPartGUIServant( KPart *part, const QDomDocument &document )
  : QObject( part )
{
  m_part = part;
  m_doc = document;
}

QAction *KPartGUIServant::action( const QDomElement &element )
{
  return m_part->action( element );
}

QDomDocument KPartGUIServant::document()
{
  return m_doc;
}

#include "kpart.moc"

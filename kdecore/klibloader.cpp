#include "klibloader.h"
#include "kglobal.h"
#include "kstddirs.h"

template QAsciiDict<KLibrary>;

#include <qtimer.h>

KLibFactory::KLibFactory( QObject* parent, const char* name )
    : QObject( parent, name )
{
}

KLibFactory::~KLibFactory()
{
}

QObject* KLibFactory::create( ClassType type, QObject* parent, const char* name, const QStringList &args )
{
    switch( type )
    {
    case Object:
	return create( parent, name, "QObject", args );
    case Widget:
	return create( parent, name, "Widget", args );
    case Part:
	return create( parent, name, "Part", args );
    case KofficeDocument:
	return create( parent, name, "KofficeDocument", args );
    }
    return 0L;
}

// -----------------------------------------------

KLibrary::KLibrary( const QString& libname, const QString& filename, lt_dlhandle handle )
{
    m_libname = libname;
    m_filename = filename;
    m_handle = handle;
    m_factory = 0;
    m_timer = 0;
}

KLibrary::~KLibrary()
{
    if ( m_factory )
      delete m_factory;
      
    lt_dlclose( m_handle );
}

QString KLibrary::name() const
{
    return m_libname;
}

QString KLibrary::fileName() const
{
    return m_filename;
}

KLibFactory* KLibrary::factory()
{
    if ( m_factory )
	return m_factory;

    QCString symname;
    symname.sprintf("init_%s", name().latin1() );

    void* sym = symbol( symname );
    if ( !sym )
	return 0;
    
    typedef KLibFactory* (*t_func)();
    t_func func = (t_func)sym;
    m_factory = func();

    if( !m_factory )
    {
	qDebug("KLibrary: The library does not offer a KDE compatible factory");
	return 0;
    }

    connect( m_factory, SIGNAL( objectCreated( QObject * ) ),
             this, SLOT( slotObjectCreated( QObject * ) ) );

    return m_factory;
}

void* KLibrary::symbol( const char* symname )
{
    void* sym = lt_dlsym( m_handle, symname );
    if ( !sym )
    {
	qDebug("KLibrary: %s", lt_dlerror() );
	return 0;
    }

    return sym;
}

void KLibrary::slotObjectCreated( QObject *obj )
{
  if ( !obj )
    return;
    
  if ( m_timer && m_timer->isActive() )
    m_timer->stop();
    
  connect( obj, SIGNAL( destroyed() ),
           this, SLOT( slotObjectDestroyed() ) );

  m_objs.append( obj );
}

void KLibrary::slotObjectDestroyed()
{
  m_objs.removeRef( sender() );
  
  if ( m_objs.count() == 0 )
  {
    qDebug( "shutdown timer started!" );
    
    if ( !m_timer )
    {
      m_timer = new QTimer( this, "klibrary_shutdown_timer" );
      connect( m_timer, SIGNAL( timeout() ),
               this, SLOT( slotTimeout() ) );
    }      
    
    m_timer->start( 1000*60, true );
  }
}

void KLibrary::slotTimeout()
{
  if ( m_objs.count() != 0 )
    return;
    
  KLibLoader::self()->unloadLibrary( m_libname );
}

// -------------------------------------------------

KLibLoader* KLibLoader::s_self = 0;

KLibLoader* KLibLoader::self()
{
    if ( !s_self )
	s_self = new KLibLoader;
    return s_self;
}

KLibLoader::KLibLoader( QObject* parent, const char* name )
    : QObject( parent, name )
{
    s_self = this;
}

KLibLoader::~KLibLoader()
{
    m_libs.setAutoDelete( TRUE );
}

KLibrary* KLibLoader::library( const char* name )
{
    QCString libname( name );
    libname += ".la";

    KLibrary* lib = m_libs[ libname ];
    if ( lib )
	return lib;

    QString libfile = KGlobal::dirs()->findResource( "lib", libname );
    if ( libfile.isEmpty() )
    {
	qDebug("KLibLoader: library=%s: No file names %s found in paths.", name, libname.data() );
	return 0;
    }
    
    lt_dlhandle handle = lt_dlopen( libfile );
    if ( !handle )
    {
	qDebug("KLibLoader: library=%s: file=%s: %s", name, libfile.latin1(), lt_dlerror() );
	return 0;
    }

    lib = new KLibrary( name, libfile, handle );
    m_libs.insert( name, lib );
    
    return lib;
}

void KLibLoader::unloadLibrary( const char *libname )
{
  KLibrary *lib = m_libs[ libname ];
  
  if ( !lib )
    return;
    
  qDebug( "closing library %s", libname );
  
  m_libs.remove( libname );
  delete lib;
}

KLibFactory* KLibLoader::factory( const char* name )
{
    KLibrary* lib = library( name );
    if ( !lib )
	return 0;
    
    return lib->factory();
}

#include "klibloader.moc"

#include "klibloader.h"
#include "kglobal.h"
#include "kstddirs.h"

KLibFactory::KLibFactory( QObject* parent, const char* name )
    : QObject( parent, name )
{
}

KLibFactory::~KLibFactory()
{
}

QObject* KLibFactory::create( ClassType type, QObject* parent, const char* name )
{
    switch( type )
    {
    case Object:
	return create( parent, name, "QObject" );
    case Widget:
	return create( parent, name, "Widget" );
    case Part:
	return create( parent, name, "Part" );
    case KofficeDocument:
	return create( parent, name, "KofficeDocument" );
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
}

KLibrary::~KLibrary()
{
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

KLibFactory* KLibLoader::factory( const char* name )
{
    KLibrary* lib = library( name );
    if ( !lib )
	return 0;
    
    return lib->factory();
}

#include "klibloader.moc"

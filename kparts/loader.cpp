#include "loader.h"

#include <dlfcn.h>

Factory::Factory( QObject* parent, const char* name )
    : QObject( parent, name )
{
}

Factory::~Factory()
{
}

// -----------------------------------------------

Library::Library( const char* libname, void* handel, Factory* factory )
    : QShared()
{
    m_libname = libname;
    m_factory = factory;
    m_handel = handel;
}

Library::~Library()
{
    dlclose( m_handel );
}

const char* Library::name() const
{
    return m_libname;
}

Factory* Library::factory()
{
    return m_factory;
}

// -------------------------------------------------

Loader* Loader::s_self = 0;

Loader* Loader::self()
{
    if ( !s_self )
	s_self = new Loader;
    return s_self;
}

Loader::Loader( QObject* parent, const char* name )
    : QObject( parent, name )
{
    s_self = this;
}

Loader::~Loader()
{
    m_libs.setAutoDelete( TRUE );
}

Factory* Loader::factory( const char* name, int major, int minor, const QStringList& deps )
{
    QCString libname;
    libname.sprintf( "%s.so.%i.%i.0", name, major, minor );

    Library* lib = m_libs[ libname ];
    if ( lib )
	return lib->factory();

    // Lets load all libs on which our target library depends.
    QStringList::ConstIterator it = deps.begin();
    for( ; it != deps.end(); ++it )
    {
	void* handel = dlopen( (*it).latin1(), RTLD_GLOBAL | RTLD_LAZY /* | RTLD_NOW */);
	if ( !handel )
	{
	    qDebug("Loader %s: %s", (*it).latin1(), dlerror() );
	    return 0;
	}	
    }
    
    void* handel = dlopen( libname, RTLD_GLOBAL | RTLD_LAZY /* | RTLD_NOW */);
    if ( !handel )
    {
	qDebug("Loader: %s", dlerror() );
	return 0;
    }

    QCString symname;
    symname.sprintf("init_%s", name );
    void* sym = dlsym( handel, symname );
    if ( !sym )
    {
	qDebug("Loader: %s", dlerror() );
	return 0;
    }

    typedef Factory* (*t_func)();
    t_func func = (t_func)sym;
    Factory* factory = func();

    if( !factory )
    {
	qDebug("Loader: The library could not be initialized");
	return 0;
    }

    lib = new Library( libname, handel, factory );
    m_libs.insert( name, lib );

    return factory;
}

#include "loader.moc"

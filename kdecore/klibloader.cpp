/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "klibloader.h"
#include "kglobal.h"
#include "kstddirs.h"
#include "kdebug.h"

template class QAsciiDict<KLibrary>;

#include <qtimer.h>
#include <qobjectdict.h>

class KLibFactoryPrivate {
public:
    const char **m_mocClasses;
};

KLibFactory::KLibFactory( QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new KLibFactoryPrivate;
    d->m_mocClasses = 0;
}

KLibFactory::~KLibFactory()
{
}

void KLibFactory::setMocClasses( const char **names )
{
    d->m_mocClasses = names;
}

void KLibFactory::destroyMocClasses() const
{
    if (objectDict && d->m_mocClasses) {
        const char **n = d->m_mocClasses;
	kdDebug(150) << "KLibFactory: removing references to meta classes" << endl;
	for (; *n; n++) {
	    kdDebug(150) << "KLibFactory: removing metaclass for " << *n << endl;
	    /* Because objectDict was set to AutoDelete the following does the
	       right thing. */
	    while (objectDict->remove(*n)) ;
	}
	kdDebug(150) << "KLibFactory: done removing meta classes" << endl;
    }
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
    if ( m_timer && m_timer->isActive() )
      m_timer->stop();

    if ( m_factory ) {
      m_factory->destroyMocClasses();
      delete m_factory;
    }

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
	kdDebug(150) << "KLibrary: The library does not offer a KDE compatible factory" << endl;
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
	kdDebug(150) << "KLibrary: " << lt_dlerror() << endl;
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
    kdDebug(150) << "KLibrary: shutdown timer for " << name() << " started!"
                 << endl;

    if ( !m_timer )
    {
      m_timer = new QTimer( this, "klibrary_shutdown_timer" );
      connect( m_timer, SIGNAL( timeout() ),
               this, SLOT( slotTimeout() ) );
    }

    // as long as it's not stable make the timeout short, for debugging
    // pleasure (matz)
    //m_timer->start( 1000*60, true );
    m_timer->start( 1000*10, true );
  }
}

void KLibrary::slotTimeout()
{
  if ( m_objs.count() != 0 )
    return;

  KLibLoader::self()->unloadLibrary( m_libname.latin1() );
}

// -------------------------------------------------

KLibLoader* KLibLoader::s_self = 0;

KLibLoader* KLibLoader::self()
{
    if ( !s_self )
	s_self = new KLibLoader;
    return s_self;
}

void KLibLoader::cleanUp()
{
  if ( !s_self )
    return;

  delete s_self;
  s_self = 0;
}

KLibLoader::KLibLoader( QObject* parent, const char* name )
    : QObject( parent, name )
{
    s_self = this;
}

KLibLoader::~KLibLoader()
{
    m_libs.setAutoDelete( TRUE );

    QAsciiDictIterator<KLibrary> it( m_libs );
    for (; it.current(); ++it )
      disconnect( it.current(), SIGNAL( destroyed() ),
	 	  this, SLOT( slotLibraryDestroyed() ) );


}

KLibrary* KLibLoader::library( const char *name )
{
    if (!name)
	return 0;

    QCString libname( name );

    // only append ".la" if there is no extension
    // this allows to load non-libtool libraries as well
    // (mhk, 20000228)
    int pos = libname.findRev('/');
    if (pos < 0)
      pos = 0;
    if (libname.find('.', pos) < 0)
      libname += ".la";

    KLibrary* lib = m_libs[ name ];
    if ( lib )
	return lib;

    // only look up the file if it is not an absolute filename
    // (mhk, 20000228)
    QString libfile;
    if (libname[0] == '/')
      libfile = libname;
    else
      {
	libfile = KGlobal::dirs()->findResource( "lib", libname );
	if ( libfile.isEmpty() )
	  {
	    kdDebug(150) << "library=" << name << ": No file names " << libname.data() << " found in paths." << endl;
	    return 0;
	  }
      }

    lt_dlhandle handle = lt_dlopen( libfile.latin1() );
    if ( !handle )
    {
	kdDebug(150) << "library=" << name << ": file=" << libfile << ": " << lt_dlerror() << endl;
	return 0;
    }

    lib = new KLibrary( name, libfile, handle );
    m_libs.insert( name, lib );

    connect( lib, SIGNAL( destroyed() ),
	     this, SLOT( slotLibraryDestroyed() ) );

    return lib;
}

void KLibLoader::unloadLibrary( const char *libname )
{
  KLibrary *lib = m_libs[ libname ];

  if ( !lib )
    return;

  kdDebug(150) << "closing library " << libname << endl;

  m_libs.remove( libname );

  disconnect( lib, SIGNAL( destroyed() ),
	      this, SLOT( slotLibraryDestroyed() ) );

  delete lib;
}

KLibFactory* KLibLoader::factory( const char* name )
{
    KLibrary* lib = library( name );
    if ( !lib )
	return 0;

    return lib->factory();
}

void KLibLoader::slotLibraryDestroyed()
{
  const KLibrary *lib = static_cast<const KLibrary *>( sender() );

  QAsciiDictIterator<KLibrary> it( m_libs );
  for (; it.current(); ++it )
    if ( it.current() == lib )
    {
      m_libs.remove( it.currentKey() );
      return;
    }
}

#include "klibloader.moc"

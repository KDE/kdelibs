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
#include <config.h>
#include <qclipboard.h>
#include <qwidgetlist.h>
#include <qwidget.h>
#include "kapp.h"
#include "klibloader.h"
#include "kglobal.h"
#include "kstddirs.h"
#include "kdebug.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

template class QAsciiDict<KLibrary>;

#include <qtimer.h>
#include <qobjectdict.h>


class KLibFactoryPrivate {
public:
};

KLibFactory::KLibFactory( QObject* parent, const char* name )
    : QObject( parent, name )
{
    d = new KLibFactoryPrivate;
}

KLibFactory::~KLibFactory()
{
    kdDebug(150) << "Deleting KLibFactory " << this << endl;
    delete d;
}

QObject* KLibFactory::create( QObject* parent, const char* name, const char* classname, const QStringList &args )
{
    QObject* obj = createObject( parent, name, classname, args );
    if ( obj )
	emit objectCreated( obj );
    return obj;
}


QObject* KLibFactory::createObject( QObject*, const char*, const char*, const QStringList &)
{
    return 0;
}


// Matz: implement all that cruft in KDE > 2.0
typedef QValueList<lt_dlhandle> LTList;

static LTList *loaded_stack = 0;
static LTList *pending_close = 0;

static void add_pending(lt_dlhandle h)
{
  if (getenv("KDE_DLCLOSE") != NULL)
    return;

  if (pending_close == 0) {
    pending_close = new LTList;
  }
  kdDebug(150) << "add pending close " << h << endl;
  pending_close->append(h);
}

static void add_loaded(lt_dlhandle h)
{
  if (getenv("KDE_DLCLOSE") != NULL)
    return;

  if (loaded_stack == 0) {
    loaded_stack = new LTList;
  }
  kdDebug(150) << "add loaded lib " << h << endl;
  loaded_stack->prepend(h);
  if (pending_close != 0)
    pending_close->remove(h);
}

static void try_close()
{
  if (loaded_stack == 0 || pending_close == 0) return;
  while (!loaded_stack->isEmpty()) {
    lt_dlhandle h = (*loaded_stack)[0];
    if (pending_close->contains(h) != 0) {
      kdDebug(150) << "try to dlclose " << h << ": yes, done." << endl;
      lt_dlclose(h);
      pending_close->remove(h);
      loaded_stack->remove(h);
    } else {
      kdDebug(150) << "try to dlclose " << h << ": not yet." << endl;
      break;
    }
  }
  if (pending_close->isEmpty()) {
    delete pending_close;
    pending_close = 0;
  }
  if (loaded_stack->isEmpty()) {
    delete loaded_stack;
    loaded_stack = 0;
  }
}

static void close_all()
{
  if (getenv("KDE_NOUNLOAD") != NULL) {
    if (loaded_stack != 0)
      delete loaded_stack;
    loaded_stack = 0;
    /* Don't need to cleanup pending_close, as it isn't initialized at all
       in KDE_NOUNLOAD case.  */
    return;
  }
  if (loaded_stack != 0) {
    LTList::ConstIterator it;
    for (it = loaded_stack->begin(); it != loaded_stack->end(); ++it) {
      lt_dlhandle h = *it;
      kdDebug(150) << "finally close lib " << h << endl;
      lt_dlclose( h );
    }
    delete loaded_stack;
    loaded_stack = 0;
  }
  if (pending_close != 0) {
    delete pending_close;
    pending_close = 0;
  }
}

// -----------------------------------------------

class KLibraryPrivate {
public:
    bool do_not_unload;
};

KLibrary::KLibrary( const QString& libname, const QString& filename, lt_dlhandle handle )
{
    m_libname = libname;
    m_filename = filename;
    m_handle = handle;
    m_factory = 0;
    m_timer = 0;
    d = new KLibraryPrivate;
    d->do_not_unload = false;
    if (lt_dlsym(handle, "__kde_do_not_unload") != 0) {
        kdDebug(150) << "Will not unload " << libname << endl;
        d->do_not_unload = true;
    }
}

KLibrary::~KLibrary()
{
    kdDebug(150) << "Deleting KLibrary " << this << "  " << m_libname << endl;
    if ( m_timer && m_timer->isActive() )
	m_timer->stop();

    // If any object is remaining, delete
    if ( m_objs.count() > 0 )
	{
	    QListIterator<QObject> it( m_objs );
	    for ( ; it.current() ; ++it )
		{
		    kdDebug(150) << "Factory still has object " << it.current() << " " << it.current()->name () << endl;
		    disconnect( it.current(), SIGNAL( destroyed() ),
				this, SLOT( slotObjectDestroyed() ) );
		}
	    m_objs.setAutoDelete(true);
	    m_objs.clear();
	}

    if ( m_factory ) {
	kdDebug(150) << " ... deleting the factory " << m_factory << endl;
	delete m_factory;
    }

    // WABA: *HACK*
    // We need to make sure to clear the clipboard before unloading a DSO
    // because the DSO could have defined an object derived from QMimeSource
    // and placed that on the clipboard.
    /*kapp->clipboard()->clear();*/

    // Well.. let's do something more subtle... convert the clipboard context
    // to text. That should be safe as it only uses objects defined by Qt.

    QWidgetList *widgetlist = QApplication::topLevelWidgets();
    QWidget *co = widgetlist->first();
    while (co) {
	if (qstrcmp(co->name(), "internal clipboard owner") == 0) {
	    if (XGetSelectionOwner(co->x11Display(), XA_PRIMARY) == co->winId())
		kapp->clipboard()->setText(kapp->clipboard()->text());
	
	    break;
	}

	co = widgetlist->next();
    }

    if (!d->do_not_unload && getenv("KDE_NOUNLOAD")==NULL) {
        if (getenv("KDE_DLCLOSE") != NULL)
            lt_dlclose( m_handle );
        else
            add_pending( m_handle );
    }

    try_close();
    delete d;
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
  
  if ( m_objs.containsRef( obj ) )
      return; // we know this object already

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
  close_all();
}

KLibLoader::KLibLoader( QObject* parent, const char* name )
    : QObject( parent, name )
{
    s_self = this;
    lt_dlinit();
}

KLibLoader::~KLibLoader()
{
    kdDebug(150) << "Deleting KLibLoader " << this << "  " << name() << endl;
    m_libs.setAutoDelete( TRUE );

    QAsciiDictIterator<KLibrary> it( m_libs );
    for (; it.current(); ++it )
    {
      kdDebug(150) << "The KLibLoader contains the library " << it.current() << endl;
      disconnect( it.current(), SIGNAL( destroyed() ),
                  this, SLOT( slotLibraryDestroyed() ) );
    }
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

    add_loaded( handle );

    if (lt_dlsym(handle, "__kde_do_not_unload") != 0) {
        /* matz: bad hack: clear loaded stack, as we can't dlclose() any
           libraries loaded before this one.  */
        delete loaded_stack;
        loaded_stack = 0;
    }

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

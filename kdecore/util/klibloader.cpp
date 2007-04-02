/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Michael Matz <matz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "config.h"
#include "klibloader.h"

#include <qclipboard.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>
#include <qlibrary.h>
#include <QStack>
#include <qapplication.h>

#include "kstandarddirs.h"
#include "kcomponentdata.h"
#include "kdebug.h"
#include "klocale.h"

#include <stdlib.h> //getenv


namespace KLibLoader_cpp {
enum UnloadMode {UNKNOWN, UNLOAD, DONT_UNLOAD};
}

/* This helper class is needed, because KLibraries can go away without
   being unloaded. So we need some info about KLibraries even after its
   death. */
class KLibWrapPrivate
{
public:
    KLibWrapPrivate(KLibrary *l, QLibrary* h);

    KLibrary *lib;
    QLibrary* handle;
    KLibLoader_cpp::UnloadMode unload_mode;
    int ref_count;
    QString name;
    QString filename;
};

class KLibLoaderPrivate
{
public:
    KLibLoaderPrivate()
        : unload_mode(KLibLoader_cpp::UNKNOWN)
    {
        if (getenv("KDE_NOUNLOAD") != 0)
            unload_mode = KLibLoader_cpp::DONT_UNLOAD;
        else if (getenv("KDE_DOUNLOAD") != 0)
            unload_mode = KLibLoader_cpp::UNLOAD;
    }

    ~KLibLoaderPrivate()
    {
        for (QHash<QString, KLibWrapPrivate*>::Iterator it = m_libs.begin(); it != m_libs.end(); ++it)
        {
            Q_ASSERT((*it) != 0);
            kDebug(150) << "The KLibLoader contains the library " << (*it)->name
                << " (" << (*it)->lib << ")" << endl;
            pending_close.append(*it);
        }

        close_pending(0);

        qDeleteAll(loaded_stack);
    }
    QStack<KLibWrapPrivate*> loaded_stack;
    QList<KLibWrapPrivate*> pending_close;
    KLibLoader_cpp::UnloadMode unload_mode;

    QString errorMessage;
    QHash<QString, KLibWrapPrivate*> m_libs;

    KLibLoader instance;

    void close_pending(KLibWrapPrivate *);
};

K_GLOBAL_STATIC(KLibLoaderPrivate, kLibLoaderPrivate)

// -------------------------------------------------------------------------

KLibFactory::KLibFactory( QObject* _parent )
    : QObject( _parent ), d(0)
{
}

KLibFactory::~KLibFactory()
{
//    kDebug(150) << "Deleting KLibFactory " << this << endl;
}

QObject* KLibFactory::create( QObject* _parent, const char* classname, const QStringList &args )
{
    QObject* obj = createObject( _parent, classname, args );
    if ( obj )
	emit objectCreated( obj );
    return obj;
}


// -----------------------------------------------

class KLibraryPrivate
{
public:
    inline KLibraryPrivate(KLibrary *_q) : q(_q) {}
    KLibrary *q;
    QString libname;
    QString filename;
    QHash<QByteArray, KLibFactory*> factories;
    QLibrary* handle;
    QList<QObject*> objs;
    QTimer* timer;

    KLibFactory *kde3Factory(const QByteArray &factoryname);
    KLibFactory *kde4Factory();
};

KLibrary::KLibrary( const QString& libname, const QString& filename, QLibrary * handle )
    : d(new KLibraryPrivate(this))
{
    /* Make sure, we have a KLibLoader */
    (void) KLibLoader::self();
    d->libname = libname;
    d->filename = filename;
    d->handle = handle;
    d->timer = 0;
}

KLibrary::~KLibrary()
{
//    kDebug(150) << "Deleting KLibrary " << this << "  " << d->libname << endl;
    if ( d->timer && d->timer->isActive() )
	d->timer->stop();

    // If any object is remaining, delete
    if ( !d->objs.isEmpty() )
	{
	    while (!d->objs.isEmpty())
		{
		    QObject *obj = d->objs.takeFirst();
		    kDebug(150) << "Factory still has object " << obj << " " << obj->objectName() << " Library = " << d->libname << endl;
		    disconnect( obj, SIGNAL( destroyed() ),
				this, SLOT( slotObjectDestroyed() ) );
		    delete obj;
		}
	}

    qDeleteAll( d->factories );
    d->factories.clear();
    delete d;
}

QString KLibrary::name() const
{
    return d->libname;
}

QString KLibrary::fileName() const
{
    return d->filename;
}

KLibFactory* KLibraryPrivate::kde3Factory(const QByteArray &factoryname)
{
    QByteArray symname = "init_";
    if(!factoryname.isEmpty()) {
        symname += factoryname;
    } else {
        symname += libname.toLatin1();
    }

    if ( factories.contains( symname ) )
        return factories[ symname ];

    KLibrary::void_function_ptr sym = q->resolveFunction( symname );
    if ( !sym )
    {
        kLibLoaderPrivate->errorMessage = i18n( "The library %1 does not offer an %2 function.", libname, QLatin1String("init_") + libname );
        kDebug(150) << kLibLoaderPrivate->errorMessage << endl;
        return 0;
    }

    typedef KLibFactory* (*t_func)();
    // Cast the void* to non-pointer type first - it's not legal to
    // cast a pointer-to-object directly to a pointer-to-function.
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(sym);
    t_func func = reinterpret_cast<t_func>(tmp);
    KLibFactory* factory = func();

    if( !factory )
    {
        kLibLoaderPrivate->errorMessage = i18n("The library %1 does not offer a KDE compatible factory." , libname);
        kDebug(150) << kLibLoaderPrivate->errorMessage << endl;
        return 0;
    }
    factories.insert( symname, factory );

    return factory;
}

KLibFactory *KLibraryPrivate::kde4Factory()
{
    const QByteArray symname("qt_plugin_instance");
    if ( factories.contains( symname ) )
        return factories[ symname ];

    KLibrary::void_function_ptr sym = q->resolveFunction( symname );
    if ( !sym )
    {
//        KLibLoader::self()->d->errorMessage = i18n("The library %1 does not offer an qt_plugin_instance function.", libname);
//        kDebug(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }

    typedef QObject* (*t_func)();
    // Cast the void* to non-pointer type first - it's not legal to
    // cast a pointer-to-object directly to a pointer-to-function.
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(sym);
    t_func func = reinterpret_cast<t_func>(tmp);
    QObject* instance = func();
    KLibFactory *factory = qobject_cast<KLibFactory *>(instance);

    if( !factory )
    {
//        KLibLoader::self()->d->errorMessage = i18n("The library %1 does not offer a KDE 4 compatible factory." , libname);
//        kDebug(150) << KLibLoader::self()->d->errorMessage << endl;
        return 0;
    }
    factories.insert( symname, factory );

    return factory;

}

KLibFactory* KLibrary::factory(const char* factoryname)
{
    KLibFactory *factory = d->kde4Factory();
    if (!factory)
        factory = d->kde3Factory(factoryname);

    if (!factory)
        return 0;

    connect(factory, SIGNAL(objectCreated(QObject *)),
             this, SLOT(slotObjectCreated(QObject * )));

    return factory;
}

void *KLibrary::resolveSymbol( const char* symname ) const
{
    void *sym = d->handle->resolve( symname );
    if ( !d->handle->isLoaded() || !sym )
    {
        kLibLoaderPrivate->errorMessage = QLatin1String("KLibrary: ") + d->handle->errorString();
        //kDebug(150) << kLibLoaderPrivate->errorMessage << endl;
        return 0;
    }

    return sym;
}

KLibrary::void_function_ptr KLibrary::resolveFunction( const char* symname ) const
{
    void *psym = d->handle->resolve( symname );
    if (!psym)
        return 0;

    // Cast the void* to non-pointer type first - it's not legal to
    // cast a pointer-to-object directly to a pointer-to-function.
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(psym);
    void_function_ptr sym = reinterpret_cast<void_function_ptr>(tmp);

    if ( !d->handle->isLoaded() || !sym )
    {
        kLibLoaderPrivate->errorMessage = QLatin1String("KLibrary: ") + d->handle->errorString();
        //kDebug(150) << kLibLoaderPrivate->errorMessage << endl;
        return 0;
    }

    return sym;
}

void KLibrary::unload() const
{
   if (!kLibLoaderPrivate.isDestroyed()) {
      kLibLoaderPrivate->instance.unloadLibrary(QFile::encodeName(name()));
   }
}

void KLibrary::slotObjectCreated( QObject *obj )
{
  if ( !obj )
    return;

  if ( d->timer && d->timer->isActive() )
    d->timer->stop();

  if ( d->objs.contains( obj ) )
      return; // we know this object already

  connect( obj, SIGNAL( destroyed() ),
           this, SLOT( slotObjectDestroyed() ) );

  d->objs.append( obj );
}

void KLibrary::slotObjectDestroyed()
{
  d->objs.removeAll( sender() );

  if ( d->objs.count() == 0 )
  {
//    kDebug(150) << "KLibrary: shutdown timer for " << name() << " started!"
//                 << endl;

      if (!QCoreApplication::instance()) {
          // Timers do not work anymore. Deleting ourselves right now.
          delete this;
          return;
      }
    if ( !d->timer )
    {
      d->timer = new QTimer( this );
      d->timer->setObjectName( QLatin1String("klibrary_shutdown_timer") );
      d->timer->setSingleShot(true);
      connect( d->timer, SIGNAL( timeout() ),
               this, SLOT( slotTimeout() ) );
    }

    // as long as it's not stable make the timeout short, for debugging
    // pleasure (matz)
    //d->timer->start( 1000*60 );
    d->timer->start( 1000*10 );
  }
}

void KLibrary::slotTimeout()
{
  if ( d->objs.count() != 0 )
    return;

  /* Don't go through KLibLoader::unloadLibrary(), because that uses the
     ref counter, but this timeout means to unconditionally close this library
     The destroyed() signal will take care to remove us from all lists.
  */
  delete this;
}

// -------------------------------------------------

KLibWrapPrivate::KLibWrapPrivate(KLibrary *l, QLibrary* h)
 : lib(l), handle(h),ref_count(1), name(l->name()), filename(l->fileName())
{
    unload_mode = KLibLoader_cpp::UNKNOWN;
    if (handle->resolve("__kde_do_not_unload") != 0) {
//        kDebug(150) << "Will not unload " << name << endl;
        unload_mode = KLibLoader_cpp::DONT_UNLOAD;
    } else if (handle->resolve("__kde_do_unload") != 0) {
        unload_mode = KLibLoader_cpp::UNLOAD;
    }
}

#define KLIBLOADER_PRIVATE KLibLoaderPrivate *const d = kLibLoaderPrivate

KLibLoader* KLibLoader::self()
{
    return &kLibLoaderPrivate->instance;
}

KLibLoader::KLibLoader( QObject* _parent )
    : QObject(_parent)
{
}

KLibLoader::~KLibLoader()
{
}

static inline QByteArray makeLibName( const char* name )
{
    QByteArray libname(name);
    int pos = libname.lastIndexOf('/');
    if (pos < 0)
      pos = 0;
    if (libname.indexOf('.', pos) < 0) {
        const char* const extList[] = { ".so", ".dylib", ".bundle", ".dll", ".sl" };
        for (uint i = 0; i < sizeof(extList) / sizeof(*extList); ++i) {
           if (QLibrary::isLibrary(libname + extList[i]))
               return libname + extList[i];
        }
    }
    return libname;
}

static inline QString findLibraryInternal(const char *name, const KComponentData &cData)
{
    QByteArray libname = makeLibName( name );

    // only look up the file if it is not an absolute filename
    // (mhk, 20000228)
    QString libfile;
    if (!QDir::isRelativePath(libname)) {
      libfile = QFile::decodeName( libname );
    }
    else
    {
      libfile = cData.dirs()->findResource("module", libname);
      if ( libfile.isEmpty() )
      {
        libfile = cData.dirs()->findResource("lib", libname);
#ifndef NDEBUG
        if ( !libfile.isEmpty() && libname.startsWith( "lib" ) ) // don't warn for kdeinit modules
          kDebug(150) << "library " << libname << " not found under 'module' but under 'lib'" << endl;
#endif
      }
    }
    return libfile;
}

//static
QString KLibLoader::findLibrary(const char *_name, const KComponentData &cData)
{
#ifndef Q_OS_WIN
    return findLibraryInternal(_name, cData);
#else
    QByteArray name( _name );
    name = name.replace( '\\', '/' );

    QString libname = findLibraryInternal(name, cData);

    // we don't have 'lib' prefix on windows -> remove it and try again
    if( libname.isEmpty() )
    {
      QByteArray ba( name );
      QByteArray file, path;

      int pos = ba.lastIndexOf( '/' );
      if ( pos >= 0 )
      {
        file = ba.mid( pos + 1 );
        path = ba.left( pos );
        name = path + '/' + file.mid( 3 );
      }
      else
      {
        file = ba;
        name = file.mid( 3 );
      }
      if( !file.startsWith( "lib" ) )
          return libname;

      libname = findLibraryInternal(name, cData);
    }

    return libname;
#endif
}


KLibrary* KLibLoader::globalLibrary( const char *_name )
{
    return library(_name, QLibrary::ExportExternalSymbolsHint);
}


KLibrary* KLibLoader::library( const char *_name, QLibrary::LoadHints hint )
{
    if (!_name)
        return 0;

    KLIBLOADER_PRIVATE;
    if (d->m_libs.contains(_name)) {
      /* Nothing to do to load the library.  */
      d->m_libs[_name]->ref_count++;
      return d->m_libs[_name]->lib;
    }

    KLibWrapPrivate* wrap = 0;
    /* Test if this library was loaded at some time, but got
       unloaded meanwhile, whithout being dlclose()'ed.  */
    QStack<KLibWrapPrivate*>::Iterator it = d->loaded_stack.begin();
    for (; it != d->loaded_stack.end(); ++it) {
      if ((*it)->name == QLatin1String(_name))
        wrap = *it;
    }

    if (wrap) {
      d->pending_close.removeAll(wrap);
      if (!wrap->lib) {
        /* This lib only was in loaded_stack, but not in d->m_libs.  */
        wrap->lib = new KLibrary( QLatin1String(_name), wrap->filename, wrap->handle );
      }
      wrap->ref_count++;
    } else {
      QString libfile = findLibrary( _name );
      if ( libfile.isEmpty() )
      {
        const QByteArray libname ( _name );
#ifndef NDEBUG
        kDebug(150) << "library=" << _name << ": No file named " << libname << " found in paths." << endl;
#endif
        d->errorMessage = i18n("Library files for \"%1\" not found in paths.",  QString(libname) );
        return 0;
      }

      QLibrary* handle = new QLibrary(libfile);
      handle->setLoadHints(hint);
      if ( !handle->load() || !handle->isLoaded() )
      {
        d->errorMessage = handle->errorString();
        return 0;
      }
      else
        d->errorMessage.clear();

      KLibrary *lib = new KLibrary( QLatin1String(_name), libfile, handle);
      wrap = new KLibWrapPrivate(lib, handle);
      d->loaded_stack.push(wrap);
    }
    d->m_libs.insert( _name, wrap );

    connect( wrap->lib, SIGNAL( destroyed() ),
             this, SLOT( slotLibraryDestroyed() ) );

    return wrap->lib;
}

QString KLibLoader::lastErrorMessage() const
{
    KLIBLOADER_PRIVATE;
    return d->errorMessage;
}

void KLibLoader::unloadLibrary( const char *libname )
{
    KLIBLOADER_PRIVATE;
  if (!d->m_libs.contains(libname))
    return;

  KLibWrapPrivate *wrap = d->m_libs[ libname ];
  if (--wrap->ref_count)
    return;

//  kDebug(150) << "closing library " << libname << endl;

  d->m_libs.remove( libname );

  disconnect( wrap->lib, SIGNAL( destroyed() ),
              this, SLOT( slotLibraryDestroyed() ) );
  d->close_pending( wrap );
}

KLibFactory* KLibLoader::factory( const char* _name, QLibrary::LoadHints hint )
{
    KLibrary* lib = library( _name, hint );
    if ( !lib )
        return 0;

    return lib->factory();
}

void KLibLoader::slotLibraryDestroyed()
{
    KLIBLOADER_PRIVATE;
  const KLibrary *lib = static_cast<const KLibrary *>( sender() );

  for (QHash<QString, KLibWrapPrivate*>::Iterator it = d->m_libs.begin(); it != d->m_libs.end(); ++it)
    if ( (*it)->lib == lib )
    {
      KLibWrapPrivate *wrap = *it;
      wrap->lib = 0;  /* the KLibrary object is already away */
      d->m_libs.remove( it.key() );
      d->close_pending( wrap );
      return;
    }
}

void KLibLoaderPrivate::close_pending(KLibWrapPrivate *wrap)
{
    if (wrap && !pending_close.contains(wrap)) {
        pending_close.append(wrap);
    }

    /* First delete all KLibrary objects in pending_close, but _don't_ unload
       the DSO behind it.  */
    for (QList<KLibWrapPrivate*>::Iterator it = pending_close.begin(); it != pending_close.end();
            ++it) {
        wrap = *it;
        if (wrap->lib) {
            QObject::disconnect(wrap->lib, SIGNAL(destroyed()),
                    &instance, SLOT(slotLibraryDestroyed()));
            KLibrary *to_delete = wrap->lib;
            wrap->lib = 0; // unset first, because KLibrary dtor can cause
            delete to_delete; // recursive call to close_pending()
        }
    }

  if (unload_mode == KLibLoader_cpp::DONT_UNLOAD) {
    while (!pending_close.isEmpty())
        delete pending_close.takeFirst();
    return;
  }

  bool deleted_one = false;
  while (!loaded_stack.isEmpty()) {
    wrap = loaded_stack.top();
    /* Let's first see, if we want to try to unload this lib.
       If the env. var KDE_DOUNLOAD is set, we try to unload every lib.
       If not, we look at the lib itself, and unload it only, if it exports
       the symbol __kde_do_unload. */
    if (unload_mode != KLibLoader_cpp::UNLOAD && wrap->unload_mode != KLibLoader_cpp::UNLOAD)
      break;

    /* Now ensure, that the libs are only unloaded in the reverse direction
       they were loaded.  */
    if (!pending_close.contains( wrap )) {
      if (!deleted_one)
        /* Only diagnose, if we really haven't deleted anything. */
//        kDebug(150) << "try to dlclose " << wrap->name << ": not yet" << endl;
      break;
    }

//    kDebug(150) << "try to dlclose " << wrap->name << ": yes, done." << endl;

    if ( !deleted_one ) {
      /* Only do the hack once in this loop.
         WABA: *HACK*
         We need to make sure to clear the clipboard before unloading a DSO
         because the DSO could have defined an object derived from QMimeSource
         and placed that on the clipboard. */
      /*qApp->clipboard()->clear();*/

      /* Well.. let's do something more subtle... convert the clipboard context
         to text. That should be safe as it only uses objects defined by Qt. */
      if( qApp->clipboard()->ownsSelection()) {
	qApp->clipboard()->setText(
            qApp->clipboard()->text( QClipboard::Selection ), QClipboard::Selection );
      }
      if( qApp->clipboard()->ownsClipboard()) {
	qApp->clipboard()->setText(
            qApp->clipboard()->text( QClipboard::Clipboard ), QClipboard::Clipboard );
      }
    }

    deleted_one = true;
    delete wrap->handle;
    pending_close.removeAll(wrap);
    delete loaded_stack.pop();
  }
}

QString KLibLoader::errorString( int componentLoadingError )
{
    switch ( componentLoadingError ) {
    case ErrNoServiceFound:
        return i18n( "No service matching the requirements was found" );
    case ErrServiceProvidesNoLibrary:
        return i18n( "The service provides no library, the Library key is missing in the .desktop file" );
    case ErrNoLibrary:
        return KLibLoader::self()->lastErrorMessage();
    case ErrNoFactory:
        return i18n( "The library does not export a factory for creating components" );
    case ErrNoComponent:
        return i18n( "The factory does not support creating components of the specified type" );
    default:
        return i18n( "KLibLoader: Unknown error" );
    }
}

#include "klibloader.moc"
// vim: sw=4 sts=4 et

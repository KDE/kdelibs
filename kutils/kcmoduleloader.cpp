/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

  This file is part of the KDE project

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <qfile.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kparts/componentfactory.h>

#include "kcmoduleloader.h"

KCModule* KCModuleLoader::load(const KCModuleInfo &mod, const QString &libname, KLibLoader *loader, QWidget * parent, const char * name, const QStringList & args )
{
  // attempt to load modules with ComponentFactory, only if the symbol init_<lib> exists
  // (this is because some modules, e.g. kcmkio with multiple modules in the library,
  // cannot be ported to KGenericFactory)
  KLibrary *lib = loader->library(QFile::encodeName(libname.arg(mod.library())));
  if (lib) {
    QString initSym("init_");
    initSym += libname.arg(mod.library());

    if ( lib->hasSymbol(QFile::encodeName(initSym)) )
    {
      // Reuse "lib" instead of letting createInstanceFromLibrary recreate it
      //KCModule *module = KParts::ComponentFactory::createInstanceFromLibrary<KCModule>(QFile::encodeName(libname.arg(mod.library())));
      KLibFactory *factory = lib->factory();
      if ( factory )
      {
        KCModule *module = KParts::ComponentFactory::createInstanceFromFactory<KCModule>( factory, parent, name ? name : mod.handle().latin1(), args );
        if (module)
          return module;
      }
      // else do a fallback
      kdDebug(1208) << "Unable to load module using ComponentFactory! Falling back to old loader." << endl;
    }

    // get the create_ function
    QString factory("create_%1");
    void *create = lib->symbol(QFile::encodeName(factory.arg(mod.handle())));

    if (create)
    {
      // create the module
      KCModule* (*func)(QWidget *, const char *);
      func = (KCModule* (*)(QWidget *, const char *)) create;
      return  func( parent, name ? name : mod.handle().latin1() );
    }

    lib->unload();
  }
  return 0;
}


KCModule* KCModuleLoader::loadModule(const KCModuleInfo &mod, bool withfallback, QWidget * parent, const char * name, const QStringList & args )
{
  /*
   * Simple libraries as modules are the easiest case:
   *  We just have to load the library and get the module
   *  from the factory.
   */

  if (!mod.library().isEmpty())
  {
    // get the library loader instance

    KLibLoader *loader = KLibLoader::self();

    KCModule *module = load(mod, "kcm_%1", loader, parent, name, args );
    if (!module) {
      // Only try to load libkcm_* if it exists, otherwise KLibLoader::lastErrorMessage would say
      // "libkcm_foo not found" instead of the real problem with loading kcm_foo.
      if ( !KLibLoader::findLibrary( QCString( "libkcm_" ) + QFile::encodeName( mod.library() ) ).isEmpty() )
        module = load(mod, "libkcm_%1", loader, parent, name, args );
    }
    if (module)
      return module;
  }
  else
    kdWarning() << "Module " << mod.fileName() << " doesn't specify a library!" << endl;

  /*
   * Ok, we could not load the library.
   * Try to run it as an executable.
   * This must not be done when calling from kcmshell, or you'll
   * have infinite recursion
   * (startService calls kcmshell which calls modloader which calls startService...)
   *
   */
  if(withfallback)
    KApplication::startServiceByDesktopPath(mod.fileName(), QString::null);
  return 0;
}

KCModule* KCModuleLoader::loadModule(const QString &module, QWidget *parent,
      const char *name, const QStringList & args)
{
    KService::Ptr serv = KService::serviceByDesktopName(module);
    if (!serv)
    {
        kdError(1208) << i18n("Module %1 not found!").arg(module) << endl;
        return 0;
    }

    KCModuleInfo mod(serv);

    return loadModule(mod, false, parent, name, args);
}

void KCModuleLoader::unloadModule(const KCModuleInfo &mod)
{
  // get the library loader instance
  KLibLoader *loader = KLibLoader::self();

  // try to unload the library
  QString libname("libkcm_%1");
  loader->unloadLibrary(QFile::encodeName(libname.arg(mod.library())));

  libname = "kcm_%1";
  loader->unloadLibrary(QFile::encodeName(libname.arg(mod.library())));
}

void KCModuleLoader::showLastLoaderError(QWidget *parent)
{
  KMessageBox::detailedError(parent,
      i18n("There was an error loading the module."),i18n("<qt><p>The diagnostics is:<br>%1"
        "<p>Possible reasons:</p><ul><li>An error occurred during your last "
        "KDE upgrade leaving an orphaned control module<li>You have old third party "
        "modules lying around.</ul><p>Check these points carefully and try to remove "
        "the module mentioned in the error message. If this fails, consider contacting "
        "your distributor or packager.</p></qt>")
      .arg(KLibLoader::self()->lastErrorMessage()));

}

// vim: ts=2 sw=2 et

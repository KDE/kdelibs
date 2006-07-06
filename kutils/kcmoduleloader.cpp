/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003,2004 Matthias Kretz <kretz@kde.org>
  Copyright (c) 2004 Frans Englich <frans.englich@telia.com>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klibloader.h>

#include <fixx11h.h>
#include "kcmoduleloader.h"


/***************************************************************/
/**
 * When something goes wrong in loading the module, this one
 * jumps in as a "dummy" module.
 */
class KCMError : public KCModule
{
	public:
		KCMError( const QString& msg, const QString& details, QWidget* parent )
			: KCModule( KGlobal::instance(), parent )
		{
			QVBoxLayout* topLayout = new QVBoxLayout( this );
			topLayout->addWidget( new QLabel( msg, this ) );
			topLayout->addWidget( new QLabel( details, this ) );
		}
};
/***************************************************************/




KCModule* KCModuleLoader::load(const KCModuleInfo &mod, QByteArray libprefix,
    ErrorReporting report, QWidget * parent, const QStringList& args )
{
  // get the library loader instance
  KLibLoader* loader = KLibLoader::self();
  QByteArray libname = libprefix + QFile::encodeName( mod.library() );

  KLibrary* lib = loader->library( libname );
  if (lib)
  {
    KLibFactory *factory = lib->factory( mod.handle().toLatin1() );
    if ( factory )
    {
      KCModule *module = factory->create<KCModule>( parent, args );
      if ( module )
        return module;
    }
    // else do a fallback
    kDebug(1208) << "Unable to load module using ComponentFactory. Falling back to old loader." << endl;

    // get the create_ function
    QByteArray factorymethod( "create_" );
    factorymethod += mod.handle().toLatin1();
    void *create = lib->symbol( factorymethod );

    if (create)
    {
      // create the module
      KCModule* (*func)(QWidget *, const char*);
      func = (KCModule* (*)(QWidget *, const char*)) create;
      return func( parent, mod.handle().toLatin1() );
    }
    else
    {
      QString libFileName = lib->fileName();
      lib->unload();
      return reportError( report, i18n("<qt>There was an error when loading the module '%1'.<br><br>"
          "The desktop file (%2) as well as the library (%3) was found but "
          "yet the module could not be loaded properly. Most likely "
          "the factory declaration was wrong, or the "
          "create_* function was missing.</qt>",
            mod.moduleName() ,
            mod.fileName() ,
            libFileName ),
          QString(), parent );
    }

    lib->unload();
  }
  return reportError( report, i18n("The specified library %1 could not be found.",
        mod.library() ), QString(), parent );
}

KCModule* KCModuleLoader::loadModule( const QString& module, ErrorReporting report, QWidget* parent, const QStringList& args )
{
  return loadModule( KCModuleInfo( module ), report, parent, args );
}

KCModule* KCModuleLoader::loadModule(const KCModuleInfo& mod, ErrorReporting report, QWidget* parent, const QStringList& args )
{
  /*
   * Simple libraries as modules are the easiest case:
   *  We just have to load the library and get the module
   *  from the factory.
   */

  if ( !mod.service() )
    return reportError( report,
        i18n("The module %1 could not be found.",
          mod.moduleName() ), i18n("<qt><p>The diagnostics is:<br>The desktop file %1 could not be found.</qt>", mod.fileName()), parent );
  if( mod.service()->noDisplay() )
    return reportError( report, i18n( "The module %1 is disabled.", mod.moduleName() ),
        i18n( "<qt><p>Either the hardware/software the module configures is not available or the module has been disabled by the administrator.</p></qt>" ),
        parent );

  if (!mod.library().isEmpty())
  {
    KCModule *module = load(mod, "", report, parent, args );
    /*
     * Only try to load libkcm_* if it exists, otherwise KLibLoader::lastErrorMessage would say
     * "libkcm_foo not found" instead of the real problem with loading kcm_foo.
     */
    if (!KLibLoader::findLibrary( QByteArray( "libkcm_" ) + QFile::encodeName( mod.library() ) ).isEmpty() )
      module = load(mod, "lib", report, parent, args );
    if (module)
      return module;
    return reportError( report,
        i18n("The module %1 could not be loaded.",
          mod.moduleName() ), QString(), parent );
  }

  /*
   * Ok, we could not load the library.
   * Try to run it as an executable.
   * This must not be done when calling from kcmshell, or you'll
   * have infinite recursion
   * (startService calls kcmshell which calls modloader which calls startService...)
   *
   */
  return reportError( report,
      i18n("The module %1 is not a valid configuration module.", mod.moduleName() ),
      i18n("<qt><p>The diagnostics is:<br>The desktop file %1 does not specify a library.</qt>", mod.fileName()), parent );
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
        "your distributor or packager.</p></qt>",
       KLibLoader::self()->lastErrorMessage()));

}

KCModule* KCModuleLoader::reportError( ErrorReporting report, const QString & text,
        QString details, QWidget * parent )
{
  if( details.isNull() )
    details = i18n("<qt><p>The diagnostics is:<br>%1"
        "<p>Possible reasons:</p><ul><li>An error occurred during your last "
        "KDE upgrade leaving an orphaned control module<li>You have old third party "
        "modules lying around.</ul><p>Check these points carefully and try to remove "
        "the module mentioned in the error message. If this fails, consider contacting "
        "your distributor or packager.</p></qt>", KLibLoader::self()->lastErrorMessage());
  if( report & Dialog )
    KMessageBox::detailedError( parent, text, details );
  if( report & Inline )
    return new KCMError( text, details, parent );
  return 0;
}

// vim: ts=2 sw=2 et


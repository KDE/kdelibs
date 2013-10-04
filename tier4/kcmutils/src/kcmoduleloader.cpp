/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003,2004,2006 Matthias Kretz <kretz@kde.org>
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

#include "kcmoduleloader.h"

#include <QtCore/QFile>
#include <QLabel>
#include <QLayout>

#include <klibrary.h>
#include <kpluginloader.h>
#include <QDebug>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

using namespace KCModuleLoader;

/***************************************************************/
/**
 * When something goes wrong in loading the module, this one
 * jumps in as a "dummy" module.
 */
class KCMError : public KCModule
{
	public:
		KCMError( const QString& msg, const QString& details, QWidget* parent )
			: KCModule( parent )
		{
			QVBoxLayout* topLayout = new QVBoxLayout( this );
      QLabel *lab = new QLabel( msg, this );
      lab->setWordWrap(true);
			topLayout->addWidget( lab );
      lab = new QLabel(details, this );
      lab->setWordWrap(true);
			topLayout->addWidget( lab );
		}
};
/***************************************************************/

KCModule *KCModuleLoader::loadModule(const QString &module, ErrorReporting report, QWidget *parent, const QStringList &args)
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
          mod.moduleName() ), i18n("<qt><p>The diagnosis is:<br />The desktop file %1 could not be found.</p></qt>", mod.fileName()), parent );
  if( mod.service()->noDisplay() )
    return reportError( report, i18n( "The module %1 is disabled.", mod.moduleName() ),
        i18n( "<qt><p>Either the hardware/software the module configures is not available or the module has been disabled by the administrator.</p></qt>" ),
        parent );

  if (!mod.library().isEmpty())
  {
        QString error;
        QVariantList args2;
        foreach (const QString &arg, args) {
            args2 << arg;
        }
        KCModule *module = mod.service()->createInstance<KCModule>(parent, args2, &error);
        if (module) {
            return module;
        }
//#ifndef NDEBUG
        {
            // get the create_ function
            KLibrary lib(mod.library());
            if (lib.load()) {
                KCModule *(*create)(QWidget *, const char *);
                QByteArray factorymethod("create_");
                factorymethod += mod.handle().toLatin1();
                create = reinterpret_cast<KCModule *(*)(QWidget *, const char*)>(lib.resolveFunction(factorymethod.constData()));
                if (create) {
                    return create(parent, mod.handle().toLatin1().constData());
                    qFatal("This module still uses a custom factory method (%s). This is not supported anymore. Please fix the module.", factorymethod.constData());
                } else {
                    qWarning() << "This module has no valid entry symbol at all. The reason could be that it's still using K_EXPORT_COMPONENT_FACTORY with a custom X-KDE-FactoryName which is not supported anymore";
                }
                lib.unload();
            }
        }
//#endif // NDEBUG
        return reportError(report, error, QString(), parent);
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
      i18n("<qt>The diagnosis is:<br />The desktop file %1 does not specify a library.</qt>", mod.fileName()), parent );
}


void KCModuleLoader::unloadModule(const KCModuleInfo &mod)
{
  // get the library loader instance
  KPluginLoader loader(mod.library());
  loader.unload();
}

KCModule* KCModuleLoader::reportError( ErrorReporting report, const QString & text,
        const QString &details, QWidget * parent )
{
    QString realDetails = details;
    if (realDetails.isNull()) {
        realDetails = i18n("<qt><p>Possible reasons:<ul><li>An error occurred during your last "
                "KDE upgrade leaving an orphaned control module</li><li>You have old third party "
                "modules lying around.</li></ul></p><p>Check these points carefully and try to remove "
                "the module mentioned in the error message. If this fails, consider contacting "
                "your distributor or packager.</p></qt>");
    }
    if (report & KCModuleLoader::Dialog) {
        KMessageBox::detailedError(parent, text, realDetails);
    }
    if (report & KCModuleLoader::Inline) {
        return new KCMError(text, realDetails, parent);
    }
    return 0;
}

// vim: ts=4

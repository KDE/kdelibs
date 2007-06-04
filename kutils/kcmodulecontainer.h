/* This file is part of the KDE libraries
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCMODULECONTAINER_H
#define KCMODULECONTAINER_H

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <kcmodule.h>
#include <kcmoduleloader.h>
#include <QtCore/QList>

class QWidget;

class KCModuleProxy;

/**
 * @brief KCModuleContainer is a convenience class encapsulating several KCModules.
 *
 * The KCModuleContainer class is a convenience class for organizing a multiple set
 * of KCModule. KCModuleContainer is a sub class of KCModule and builds an interface mainly
 * consisting of a tab widget where each tab contains one of the modules specified via one of the
 * constructors. KCModuleContainer can handle modules which requires root permissions. What you
 * most likely want is the KCMODULECONTAINER macro. \n
 * Sometimes it is of interest to detect in runtime whether a module should be loaded or not. This
 * can be achieved by sub classing KCModuleContainer, doing the probing/testing checks and then manually
 * call addModule for each module which should be displayed. When all calls to addModule is done, call
 * finalize() which performs some necessary final steps.
 *
 * @author Frans Englich <frans.englich@telia.com>
 */
class KUTILS_EXPORT KCModuleContainer : public KCModule
{
	Q_OBJECT
	public:
		/**
		 * Creates a KCModuleContainer with tabs, each one containing one of the
		 * specified modules in @p mods.
		 *
                 * @param parent the parent QWidget.
		 * @param mods The list of KCModules to be loaded. The name of each
		 * KCModule is its service name, that is the name of the desktop file without
		 * the ".desktop" part
		 *
		 */
		KCModuleContainer( QWidget* parent, const QStringList& mods );

		/**
		 * This is a convenience function, instead of building a QStringList you
		 * can specify the modules in a comma separated QString. For example;
		 * \code
		 * KCModuleContainer* cont = KCModuleContainer( this, "kcm_misc", QString("kcm_energy, kcm_keyboard ,kcm_useraccount, kcm_mouse") );
		 * \endcode
		 * The other constructor takes its modules in a QStringlist which also can be constructed from a
		 * string and thus you will have to be explicit on the data type.
		 *
		 * What you probably want is the KCMODULECONTAINER macro which builds an KCModule
		 * for you, taking the modules you want as argument.
		 *
		 * @param parent The parent widget
		 * @param mods The modules to load
		 * @return The KCModule containing the requested modules.
		 */
		explicit KCModuleContainer( QWidget *parent, const QString& mods = QString() );

		/**
		 * Adds the specified module to the tab widget. Setting the tab icon, text,
		 * tool tip, connecting the signals is what it does.
		 *
		 * @param module the name of the module to add. The name is the desktop file's name
		 * without the ".desktop" part.
		 */
		void addModule( const QString& module );

		/**
		 * Default destructor.
		 */
		virtual ~KCModuleContainer();

		/**
		 * Reimplemented for internal purposes.
		 * @internal
		 */
		void save();

		/**
		 * Reimplemented for internal purposes.
		 * @internal
		 */
		void load();

		/**
		 * Reimplemented for internal purposes.
		 * @internal
		 */
		void defaults();

	protected:
		typedef QList<KCModuleProxy*> ModuleList;

		/**
		 * A list containing KCModuleProxy objects which
		 * have changed and must be saved.
		 */
		ModuleList changedModules;

		/**
		 * A list of all modules which are encapsulated.
		 */
		ModuleList allModules; // KDE 4 put in the Private class and abstract with getter

	private Q_SLOTS:

		/**
		 * Enables/disables the Admin Mode button, as appropriate.
		 */
		void tabSwitched( QWidget * module );

		void moduleChanged(KCModuleProxy *proxy);

	private:

		void init();

		class KCModuleContainerPrivate;
		KCModuleContainerPrivate* const d;

};

/**
 * This macro creates an factory declaration which when run creates an KCModule with specified
 * modules. For example:
 * \code
 * KCMODULECONTAINER("kcm_fonts,kcm_keyboard,kcm_foo", misc_modules)
 * \endcode
 * would create a KCModule with three tabs, each containing one of the specified KCMs. Each
 * use of the macro must be accompanied by a desktop file where the factory name equals
 * the second argument in the macro(in this example, misc_modules). \n
 * The module container takes care of testing the contained modules when being shown, as well
 * as when the module itself is asked whether it should be shown.
 *
 * @param modules the modules to put in the container
 * @param factoryName what factory name the module should have
 */
#define KCMODULECONTAINER(modules, factoryName) \
class KCModuleContainer##factoryName : public KCModuleContainer \
{ \
    public: \
        KCModuleContainer##factoryName(QWidget *parent, const QStringList &) \
            : KCModuleContainer(parent, QLatin1String(modules)) \
        { \
        } \
}; \
typedef KGenericFactory<KCModuleContainer#factoryName> KCModuleContainer##factoryName##Factory; \
K_EXPORT_COMPONENT_FACTORY(factoryName, KCModuleContainer##factoryName##Factory)

#endif // KCMODULECONTAINER_H



/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KSETTINGS_PLUGINPAGE_H
#define KSETTINGS_PLUGINPAGE_H

#include <kcmodule.h>
#include <kdelibs_export.h>

class KPluginSelector;

namespace KSettings
{

/**
 * @short Convenience KCModule for creating a plugins config page.
 *
 * This class makes it very easy to create a plugins configuration page to your
 * program. All you need to do is create a class that is derived from
 * PluginPage and add the appropriate plugin infos to the KPluginSelector.
 * This is done using the pluginSelector() method:
 * \code
 * typedef KGenericFactory<MyAppPluginConfig, QWidget> MyAppPluginConfigFactory;
 * K_EXPORT_COMPONENT_FACTORY( kcm_myapppluginconfig, MyAppPluginConfigFactory( "kcm_myapppluginconfig" ) );
 *
 * MyAppPluginConfig( QWidget * parent, const QStringList & args )
 *     : PluginPage( MyAppPluginConfigFactory::instance(), parent, args )
 * {
 *     pluginSelector()->addPlugins( KGlobal::instance()->instanceName(), i18n( "General Plugins" ), "General" );
 *     pluginSelector()->addPlugins( KGlobal::instance()->instanceName(), i18n( "Effects" ), "Effects" );
 * }
 * \endcode
 *
 * All that remains to be done is to create the appropriate .desktop file
 * \verbatim
   [Desktop Entry]
   Encoding=UTF-8
   Icon=plugin
   Type=Service
   ServiceTypes=KCModule

   X-KDE-ModuleType=Library
   X-KDE-Library=myapppluginconfig
   X-KDE-FactoryName=MyAppPluginConfigFactory
   X-KDE-ParentApp=myapp
   X-KDE-ParentComponents=myapp

   Name=Plugins
   Comment=Select and configure your plugins:
   \endverbatim
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KUTILS_EXPORT PluginPage : public KCModule
{
    Q_OBJECT
    public:

#ifdef KDE3_SUPPORT
        /**
         * Standart KCModule constructor. Automatically creates the the
         * KPluginSelector widget.
         */
        PluginPage( QWidget * parent = 0, const char * name = 0, const QStringList & args = QStringList() ) KDE_DEPRECATED;
#endif

        /**
         * Standart KCModule constructor. Automatically creates the the
         * KPluginSelector widget.
         */
        PluginPage( KInstance * instance, QWidget * parent = 0, const QStringList & args = QStringList() );

        ~PluginPage();

        /**
         * @return a reference to the KPluginSelector.
         */
        KPluginSelector * pluginSelector();

        /**
         * Load the state of the plugins (selected or not) from the KPluginInfo
         * objects. For KParts plugins everything should work automatically. For
         * your own type of plugins you might need to reimplement the
         * KPluginInfo::pluginLoaded() method. If that doesn't fit your needs
         * you can also reimplement this method.
         */
        virtual void load();

        /**
         * Save the state of the plugins to KConfig objects
         */
        virtual void save();
        virtual void defaults();

    private:
        class PluginPagePrivate;
        PluginPagePrivate* const d;
};

}

// vim: sw=4 sts=4 et

#endif // KSETTINGS_PLUGINPAGE_H

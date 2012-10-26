/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kwalletpluginloader.h"
#include "kwalletplugin.h"
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include <kdebug.h>
#include <kservice.h>

namespace KWallet {

class WalletPluginLoader::Private {
public:
    Private( WalletPluginLoader* loader ) : m_loader( loader ) {}

    WalletPluginLoader *m_loader;
};

WalletPluginLoader::WalletPluginLoader(): 
    QObject(),
    d( new Private( this ) )
{
}

WalletPluginLoader::~WalletPluginLoader()
{
    delete d;
}

WalletPlugin* WalletPluginLoader::loadKWallet()
{
    WalletPlugin *plugin =0;
    kDebug(285) << "Loading kwallet default plugin";

    QString pathToDesktopFile = KStandardDirs::locate("services", "kwalletdefaultplugin.desktop");
    KService service(pathToDesktopFile);

    KPluginFactory *factory = KPluginLoader( service.library() ).factory();
    if ( !factory ) {
        kDebug() << "Cannot create kwalletdefaultplugin factory";
    }
    else {
        plugin = factory->create< WalletPlugin >();
        if ( plugin )
            kDebug() << "Plugin loaded";
        else {
            kDebug() << "Factory cannot create kwalletdefaultplugin instance";
        }
    }

    if ( !plugin ) {
        KMessageBox::error(0, i18n("Cannot load KWallet default plugin. Please check your installation."), "KWallet");
    }

    return plugin;
}

WalletPlugin* WalletPluginLoader::loadKSecrets()
{
    WalletPlugin *plugin =0;
    kDebug(285) << "Loading kwallet ksecrets plugin";

    QString pathToDesktopFile = KStandardDirs::locate("services", "kwallet-ksecrets-plugin.desktop");
    KService service(pathToDesktopFile);

    KPluginFactory *factory = KPluginLoader( service.library() ).factory();
    if ( !factory ) {
        kDebug() << "Cannot create kwallet ksecrets factory";
    }
    else {
        plugin = factory->create< WalletPlugin >();
        if ( plugin )
            kDebug() << "Plugin loaded";
        else {
            kDebug() << "Factory cannot create kwallet ksecrets instance";
        }
    }

    if ( !plugin ) {
        KMessageBox::error(0, i18n("Cannot load KWallet KSecrets plugin. Please check your installation or configuration."), "KWallet");
    }

    return plugin;
}

WalletPluginLoader* WalletPluginLoader::instance()
{
    static WalletPluginLoader *instance =0;
    if ( 0 == instance ) {
        instance = new WalletPluginLoader();
    }
    return instance;
}



} // namespace 

#include "kwalletpluginloader.moc"

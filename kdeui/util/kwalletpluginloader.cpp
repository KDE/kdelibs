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
    return 0;
}

WalletPlugin* WalletPluginLoader::loadKSecrets()
{
    return 0;
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

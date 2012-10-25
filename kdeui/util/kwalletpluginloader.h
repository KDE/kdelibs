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
#ifndef KWALLET_PLUGIN_LOADER_H
#define KWALLET_PLUGIN_LOADER_H


#include <qobject.h>

namespace KWallet {

class WalletPlugin;

class WalletPluginLoader : public QObject
{
    Q_OBJECT

    WalletPluginLoader();
    WalletPluginLoader(const WalletPluginLoader&);
    WalletPluginLoader operator = (const WalletPluginLoader&);

public:
    virtual ~WalletPluginLoader();
    static WalletPluginLoader* instance();
    WalletPlugin* loadKWallet();
    WalletPlugin* loadKSecrets();

private:
    class Private;
    Private * const d;
};

} // namespace

#endif // KWALLET_PLUGIN_LOADER_H

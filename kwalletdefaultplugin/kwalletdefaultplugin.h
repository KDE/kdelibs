/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
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
#ifndef KWALLET_DEFAULT_PLUGIN_H
#define KWALLET_DEFAULT_PLUGIN_H

#include <QObject>
#include "kwalletplugin.h"
#include <kcomponentdata.h>

namespace KWallet {

class WalletDefaultPluginPrivate;

class WalletDefaultPlugin : public WalletPlugin
{
    Q_OBJECT
public:
    WalletDefaultPlugin();
    explicit WalletDefaultPlugin(QObject* parent = 0);
    WalletDefaultPlugin(QObject* parent, const QVariantList& );
    virtual ~WalletDefaultPlugin();

    // BEGIN WalletPlugin abstract methods implementation
    virtual QStringList walletList() ;
    virtual bool isEnabled() ;
    virtual bool isOpen(const QString& name) ;
    virtual int closeWallet(const QString& name, bool force) ;
    virtual int deleteWallet(const QString& name) ;
    virtual bool disconnectApplication(const QString& wallet, const QString& app) ;
    virtual bool openWallet(const QString& name, WId w, OpenType ot = Synchronous) ;
    virtual QStringList users(const QString& wallet) ;
    virtual void changePassword(const QString& name, WId w) ;
    virtual int sync() ;
    virtual int lockWallet() ;
    virtual const QString& walletName() const;
    virtual bool isOpen() const ;
    virtual void requestChangePassword(WId w) ;
    virtual QStringList folderList() ;
    virtual bool hasFolder(const QString& f) ;
    virtual bool setFolder(const QString& f) ;
    virtual const QString& currentFolder() const ;
    virtual bool removeFolder(const QString& f) ;
    virtual bool createFolder(const QString& f) ;
    virtual QStringList entryList() ;
    virtual int renameEntry(const QString& oldName, const QString& newName) ;
    virtual int readEntry(const QString& key, QByteArray& value) ;
    virtual int readMap(const QString& key, QMap<QString,QString>& value) ;
    virtual int readPassword(const QString& key, QString& value) ;
    int readEntryList(const QString& key, QMap<QString, QByteArray>& value) ;
    int readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) ;
    int readPasswordList(const QString& key, QMap<QString, QString>& value) ;
    virtual int writeEntry(const QString& key, const QByteArray& value, EntryType entryType) ;
    virtual int writeEntry(const QString& key, const QByteArray& value) ;
    virtual int writeMap(const QString& key, const QMap<QString,QString>& value) ;
    virtual int writePassword(const QString& key, const QString& value) ;
    virtual bool hasEntry(const QString& key) ;
    virtual int removeEntry(const QString& key) ;
    virtual EntryType entryType(const QString& key) ;
    virtual bool folderDoesNotExist(const QString& wallet, const QString& folder) ;
    virtual bool keyDoesNotExist(const QString& wallet, const QString& folder,
                    const QString& key) ;
    // END WalletPlugin abstract methods implementation

Q_SIGNALS:
    /**
    *  Emitted when this wallet is closed.
    */
    void walletClosed();

    /**
    *  Emitted when a folder in this wallet is updated.
    *  @param folder The folder that was updated.
    */
    void folderUpdated(const QString& folder);

    /**
    *  Emitted when the folder list is changed in this wallet.
    */
    void folderListUpdated();

    /**
    *  Emitted when a folder in this wallet is removed.
    *  @param folder The folder that was removed.
    */
    void folderRemoved(const QString& folder);

    /**
    *  Emitted when a wallet is opened in asynchronous mode.
    *  @param success True if the wallet was opened successfully.
    */
    void walletOpened(bool success);

private Q_SLOTS:
    /**
    *  @internal
    *  DBUS slot for signals emitted by the wallet service.
    */
    void slotWalletClosed(int handle);

    /**
    *  @internal
    *  DBUS slot for signals emitted by the wallet service.
    */
    void slotFolderUpdated(const QString& wallet, const QString& folder);

    /**
    *  @internal
    *  DBUS slot for signals emitted by the wallet service.
    */
    void slotFolderListUpdated(const QString& wallet);

    /**
    *  @internal
    *  DBUS slot for signals emitted by the wallet service.
    */
    void slotApplicationDisconnected(const QString& wallet, const QString& application);

    /**
    *  @internal
    *  Callback for kwalletd
    *  @param tId identifer for the open transaction
    *  @param handle the wallet's handle
    */
    void walletAsyncOpened(int tId, int handle);

    /**
    *  @internal
    */
    void walletServiceUnregistered();

private:
    void connectTokwalletd();
    void emitWalletAsyncOpenError();
    void emitWalletOpened();
    
    friend class WalletDefaultPluginPrivate;
    WalletDefaultPluginPrivate *d;
};

} // namespace

#endif // KWALLET_DEFAULT_PLUGIN_H


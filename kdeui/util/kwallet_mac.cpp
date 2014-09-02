/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
 * Copyright (C) 2008 Michael Leupold <lemma@confuego.org>
 * Copyright (C) 2010 Frank Osterfeld <osterfeld@kde.org>
 * Copyright (C) 2014 René Bertin <rjvbertin@gmail.com>
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

#include "kwallet.h"
#include <ksharedconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <ktoolinvocation.h>

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kconfiggroup.h>

#include <cassert>

#include <sys/param.h>

#include "qosxkeychain.h"

using namespace KWallet;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)
typedef QMap<QString, StringStringMap> StringToStringStringMapMap;
Q_DECLARE_METATYPE(StringToStringStringMapMap)
typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

#ifdef OSX_KEYCHAIN_PORT_DISABLED
static QString appid()
{
    KComponentData cData = KGlobal::mainComponent();
    if (cData.isValid()) {
        const KAboutData* aboutData = cData.aboutData();
        if (aboutData) {
            return aboutData->programName();
        }
        return cData.componentName();
    }
    return qApp->applicationName();
}
#endif

/*static*/ const QString Wallet::LocalWallet()
{
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));
    if (!cfg.readEntry("Use One Wallet", true)) {
        QString tmp = cfg.readEntry("Local Wallet", "localwallet");
        if (tmp.isEmpty()) {
            return "localwallet";
        }
        return tmp;
    }

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return "kdewallet";
    }
    return tmp;
}

/*static*/ const QString Wallet::NetworkWallet()
{
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return "kdewallet";
    }
    return tmp;
}

/*static*/ const QString Wallet::PasswordFolder()
{
    return "Passwords";
}

/*static*/ const QString Wallet::FormDataFolder()
{
    return "Form Data";
}

#pragma mark ==== Wallet::WalletPrivate ====
class Wallet::WalletPrivate : public OSXKeychain
{
public:
    explicit WalletPrivate(const QString &n)
        : OSXKeychain(n)
    {
        isKDEChain = ( n == LocalWallet() || n == NetworkWallet() || n.contains( "wallet", Qt::CaseInsensitive ) );
    }

    // needed for compilation reasons
    void walletServiceUnregistered()
    {
    }
};

Wallet::Wallet(int handle, const QString& name)
    : QObject(0L), d(new WalletPrivate(name))
{
    Q_UNUSED(handle);
}

Wallet::~Wallet()
{
    delete d;
}

/*static*/ QStringList Wallet::walletList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().wallets();
#else
    // RJVB: Mac OS X's Keychain supports multiple keychains, but they can only be accesses by full path, not
    // found by name. That makes it cumbersome to map to multiple wallets when using only the wallet name.
    // However, it would be perfectly possible to create OS X Keychains called Wallet::LocalWallet() and
    // Wallet::NetworkWallet() in the equivalent of ~/.kde/share/apps/kwallet .
    QStringList l;
    OSXKeychain::KeychainList(l);
    return l;
#endif
}


/*static*/ void Wallet::changePassword(const QString& name, WId w)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::changePassword().";
    walletLauncher->getInterface().changePassword(name, (qlonglong)w, appid());
#else
    Q_UNUSED(w);
    kWarning() << "Wallet::changePassword unimplemented '" << name << "'";
#endif
}


/*static*/ bool Wallet::isEnabled()
{
    //PENDING(frank) check
    return true;
}


/*static*/ bool Wallet::isOpen(const QString& name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().isOpen(name); // default is false
#else
    return OSXKeychain::IsOpen(name);
#endif
}

bool Wallet::isOpen() const
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return d->handle != -1;
#else
    return d->isOpen();
#endif
}


/*static*/ int Wallet::closeWallet(const QString& name, bool force)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<int> r = walletLauncher->getInterface().close(name, force);
    return r.isValid() ? r : -1;
#else
    Q_UNUSED(force);
    return OSXKeychain::Lock(name);
#endif
}


/*static*/ int Wallet::deleteWallet(const QString& name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<int> r = walletLauncher->getInterface().deleteWallet(name);
    return r.isValid() ? r : -1;
#else
    return OSXKeychain::Destroy(name);
#endif
}


/*static*/ Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot)
{
    Q_UNUSED(w);
    Q_UNUSED(ot);
    Wallet *wallet = new Wallet(-1, name);
    QMetaObject::invokeMethod( wallet, "emitWalletOpened", Qt::QueuedConnection );
    OSStatus err = wallet->d->unLock();
    kDebug() << "Opened wallet '" << name << "': " << wallet << " error=" << err;
    return wallet;
}


/*static*/ bool Wallet::disconnectApplication(const QString& wallet, const QString& app)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().disconnectApplication(wallet, app); // default is false
#else
    kWarning() << "Wallet::disconnectApplication unimplemented, '" << app << "' from '" << wallet << "'";
    return true;
#endif
}


/*static*/ QStringList Wallet::users(const QString& name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().users(name); // default is QStringList()
#else
    kWarning() << "Wallet::users unimplemented, '" << name << "'";
    return QStringList();
#endif
}


int Wallet::sync()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return -1;
    }

    walletLauncher->getInterface().sync(d->handle, appid());
#endif
    return 0;
}


int Wallet::lockWallet()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return -1;
    }

    QDBusReply<int> r = walletLauncher->getInterface().close(d->handle, true, appid());
    d->handle = -1;
    d->folder.clear();
    d->name.clear();
    if (r.isValid()) {
        return r;
    }
#else
    d->currentService.clear();
#endif
    return d->lock();
}


const QString& Wallet::walletName() const
{
    return d->name;
}


void Wallet::requestChangePassword(WId w)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";
    if (d->handle == -1) {
        return;
    }

    walletLauncher->getInterface().changePassword(d->name, (qlonglong)w, appid());
#else
    Q_UNUSED(w);
    kWarning() << "Wallet::requestChangePassword unimplemented '" << d->name << "'";
#endif
}


void Wallet::slotWalletClosed(int handle)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == handle) {
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        emit walletClosed();
    }
#else
    Q_UNUSED(handle);
    kWarning() << "Wallet::slotWalletClosed unimplemented '" << d->name << "'";
    d->currentService.clear();
#endif
}


QStringList Wallet::folderList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().folderList(d->handle, appid());
    return r;
#else
    return QStringList(d->folderList());
#endif
}


QStringList Wallet::entryList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().entryList(d->handle, d->folder, appid());
    return r;
#else
    QStringList r = QStringList();
    d->itemList(r);
    return r;
#endif
}


bool Wallet::hasFolder(const QString& f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().hasFolder(d->handle, f, appid());
    return r; // default is false
#else
    d->folderList();
    return d->serviceList.contains(f);
#endif
}


bool Wallet::createFolder(const QString& f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = walletLauncher->getInterface().createFolder(d->handle, f, appid());
        return r;
    }

    return true;				// folder already exists
#else
    return setFolder(f);
#endif
}


bool Wallet::setFolder(const QString &f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    bool rc = false;

    if (d->handle == -1) {
        return rc;
    }

    // Don't do this - the folder could have disappeared?
#if 0
    if (f == d->folder) {
        return true;
    }
#endif

    if (hasFolder(f)) {
        d->folder = f;
        rc = true;
    }

    return rc;
#else
    // act as if we just changed folders even if we have no such things; the property
    // is stored as the ServiceItemAttr (which shows up as the "Where" field in the Keychain Utility).
    if( f.size() == 0 ){
        d->currentService.clear();
    }
    else{
        d->currentService = QString(f);
    }
    return true;
#endif
}


bool Wallet::removeFolder(const QString& f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().removeFolder(d->handle, f, appid());
    if (d->folder == f) {
        setFolder(QString());
    }

    return r;					// default is false
#else
    kWarning() << "Wallet::removeFolder unimplemented (returns true) '" << d->name << "'";
    if( d->currentService == f ){
        d->currentService.clear();
    }
    return true;
#endif
}


const QString& Wallet::currentFolder() const
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return d->folder;
#else
    return d->currentService;
#endif
}


int Wallet::readEntry(const QString &key, QByteArray &value)
{   OSStatus err = d->readItem( key, &value, NULL );
    kDebug() << "Wallet::readEntry '" << key << "' from wallet " << d->name << ", error=" << ((err)? -1 : 0);
    return (err)? -1 : 0;
}


int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = walletLauncher->getInterface().readEntryList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        // convert <QString, QVariant> to <QString, QByteArray>
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toByteArray());
        }
    }

    return rc;
#else
    Q_UNUSED(key);
    Q_UNUSED(value);
    kWarning() << "Wallet::readEntryList unimplemented (returns -1) '" << d->name << "'";
    return -1;
#endif
}


int Wallet::renameEntry(const QString& oldName, const QString& newName)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().renameEntry(d->handle, d->folder, oldName, newName, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
#else
    return d->renameItem( oldName, newName );
#endif
}


int Wallet::readMap(const QString &key, QMap<QString,QString> &value)
{
    QByteArray v;
    const int ret = (d->readItem( key, &v, NULL ))? -1 : 0;
    if( ret != 0 ){
        return ret;
    }
    if( !v.isEmpty() ){
        QByteArray w = QByteArray::fromBase64(v);
        QDataStream ds( &w, QIODevice::ReadOnly );
        ds >> value;
    }
    kDebug() << "Wallet::readMap '" << key << "' from wallet " << d->name << ", error=0";
    return 0;
}


int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r =
        walletLauncher->getInterface().readMapList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            QByteArray mapData = it.value().toByteArray();
            if (!mapData.isEmpty()) {
                QDataStream ds(&mapData, QIODevice::ReadOnly);
                QMap<QString,QString> v;
                ds >> v;
                value.insert(it.key(), v);
            }
        }
    }

    return rc;
#else
    Q_UNUSED(key);
    Q_UNUSED(value);
    kWarning() << "Wallet::readMapList unimplemented (returns -1) '" << d->name << "'";
    return -1;
#endif
}


int Wallet::readPassword(const QString& key, QString& value)
{
    QByteArray ba;
    const int ret = (d->readItem( key, &ba, NULL ))? -1 : 0;
    if ( ret == 0 ){
        value = QString::fromUtf8( ba.constData() );
    }
    kDebug() << "Wallet::readPassword '" << key << "' from wallet " << d->name << ", error=" << ret;
    return ret;
}


int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
    kWarning() << "Wallet::readPasswordList unimplemented (returns -1) '" << d->name << "'";
    return -1;
}

int Wallet::writeEntry(const QString& key, const QByteArray& password )
{   int ret = d->writeItem( key, password );
    kDebug() << "wrote entry '" << key << "' to wallet " << d->name << ", error=" << ret;
    return ret;
}

int Wallet::writeEntry(const QString& key, const QByteArray& password, EntryType entryType)
{
    OSXKeychain::EntryType entryCode;
	switch( entryType ){
		case Wallet::Password:
			entryCode = OSXKeychain::Password;
			break;
		case Wallet::Map:
			entryCode = OSXKeychain::Map;
			break;
        case Wallet::Stream:
            entryCode = OSXKeychain::Stream;
            break;
		default:
			entryCode = OSXKeychain::Unknown;
			break;
	}
	int ret = d->writeItem( key, password, &entryCode );
    kDebug() << "wrote entry '" << key << "' of type=" << (int) entryType << "to wallet " << d->name << ", error=" << ret;
    return ret;
}

int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value)
{
    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    OSXKeychain::EntryType etype = OSXKeychain::Map;
    int ret = d->writeItem( key, mapData.toBase64(),
                           "This is a KDE Wallet::Map item. Its password\n"
                           "cannot be read in the OS X Keychain Utility.\n"
                           "Use KDE's own kwalletmanager for that.", &etype );
    kDebug() << "wrote map '" << key << "' to wallet " << d->name << ", error=" << ret;
    return ret;
}


int Wallet::writePassword(const QString &key, const QString& value)
{   OSXKeychain::EntryType etype = OSXKeychain::Password;
    int ret = d->writeItem( key, value.toUtf8(), &etype );
    kDebug() << "wrote password '" << key << "' to wallet " << d->name << ", error=" << ret;
    return ret;
}


bool Wallet::hasEntry(const QString &key)
{   bool ret = d->hasItem( key, NULL );
    kDebug() << "wallet '" << d->name << "'" << ((ret)? " has" : " does not have") << " entry '" << key << "'";
    return ret;
}

int Wallet::removeEntry(const QString& key)
{   int ret = d->removeItem( key );
    kDebug() << "removed entry '" << key << "' from wallet " << d->name << ", error=" << ret;
    return ret;
}


Wallet::EntryType Wallet::entryType(const QString& key)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    int rc = 0;

    if (d->handle == -1) {
        return Wallet::Unknown;
    }

    QDBusReply<int> r = walletLauncher->getInterface().entryType(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return static_cast<EntryType>(rc);
#else
    // RJVB: a priori, entries are always 'password' on OS X, but since we also do use them for storing
    // maps, it may be best to return Wallet::Unknown to leave some uncertainty and not mislead our caller.
    OSXKeychain::EntryType etype;
    if( !d->itemType( key, &etype ) ){
        switch( etype ){
            case OSXKeychain::Password:
                return Wallet::Password;
                break;
            case OSXKeychain::Map:
                return Wallet::Map;
                break;
            case OSXKeychain::Stream:
                return Wallet::Stream;
                break;
        }
    }
    return Wallet::Unknown;
#endif
}


void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder)
{
    if (d->name == wallet) {
        emit folderUpdated(folder);
    }
}


void Wallet::slotFolderListUpdated(const QString& wallet)
{
    if (d->name == wallet) {
        emit folderListUpdated();
    }
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle >= 0
        && d->name == wallet
        && application == appid()) {
        slotWalletClosed(d->handle);
    }
#else
    Q_UNUSED(wallet);
    Q_UNUSED(application);
	kWarning() << "Wallet::slotApplicationDisconnected unimplemented '" << d->name << "'";
#endif
}

void Wallet::walletAsyncOpened(int tId, int handle)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    // ignore responses to calls other than ours
    if (d->transactionId != tId || d->handle != -1) {
        return;
    }

    // disconnect the async signal
    disconnect(this, SLOT(walletAsyncOpened(int,int)));

    d->handle = handle;
    emit walletOpened(handle > 0);
#else
    Q_UNUSED(tId);
    Q_UNUSED(handle);
	kWarning() << "Wallet::walletAsyncOpened unimplemented '" << d->name << "'";
#endif
}

void Wallet::emitWalletAsyncOpenError()
{
    emit walletOpened(false);
}

void Wallet::emitWalletOpened()
{
  emit walletOpened(true);
}


bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<bool> r = walletLauncher->getInterface().folderDoesNotExist(wallet, folder);
    return r;
#else
    bool ret = true;
    if( Wallet::walletList().contains(wallet) ){
        ret = !Wallet(-1, wallet).hasFolder(folder);
    }
    return ret;
#endif
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<bool> r = walletLauncher->getInterface().keyDoesNotExist(wallet, folder, key);
    return r;
#else
    bool ret = true;
    if( Wallet::walletList().contains(wallet) ){
        Wallet w(-1, wallet);
        if( w.hasFolder(folder) ){
            ret = !w.hasEntry(key);
        }
    }
    return ret;
#endif
}

void Wallet::slotCollectionStatusChanged(int status)
{
    Q_UNUSED(status);
	kWarning() << "Wallet::slotCollectionStatusChanged unimplemented '" << d->name << "' status=" << status;
}

void Wallet::slotCollectionDeleted()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    d->folder.clear();
#else
    d->currentService.clear();
#endif
    kDebug() << "Wallet::slotCollectionDeleted: closing private data '" << d->name;
    d->close();
    emit walletClosed();
}


void Wallet::virtual_hook(int, void*)
{
    //BASE::virtual_hook( id, data );
}

#include "kwallet.moc"

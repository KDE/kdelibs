/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
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
#include <kconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtDBus/QtDBus>
#include "kwallet_interface.h"

#include <assert.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kconfiggroup.h>

using namespace KWallet;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)
typedef QMap<QString, StringStringMap> StringToStringStringMapMap;
Q_DECLARE_METATYPE(StringToStringStringMapMap)
typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

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

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
        qDBusRegisterMetaType<StringStringMap>();
        qDBusRegisterMetaType<StringToStringStringMapMap>();
        qDBusRegisterMetaType<StringByteArrayMap>();
        registered = true;
    }
}

const QString Wallet::LocalWallet() {
    KConfig kwalletrc("kwalletrc");
    KConfigGroup cfg(&kwalletrc, "Wallet");
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

const QString Wallet::NetworkWallet() {
    KConfig kwalletrc("kwalletrc");
    KConfigGroup cfg(&kwalletrc, "Wallet");

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return "kdewallet";
    }
    return tmp;
}

const QString Wallet::PasswordFolder() {
    return "Passwords";
}

const QString Wallet::FormDataFolder() {
    return "Form Data";
}

class Wallet::WalletPrivate
{
public:
    WalletPrivate(int h, const QString &n)
     : name(n), handle(h)
    {}
    QString name;
    QString folder;
    int handle;
    OrgKdeKWalletInterface* wallet;
};


Wallet::Wallet(int handle, const QString& name)
    : QObject(0L), d(new WalletPrivate(handle, name)) {

    d->wallet = new org::kde::KWallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    d->wallet->setParent(this);

    connect(QDBusConnection::sessionBus().interface(),
            SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this,
            SLOT(slotServiceOwnerChanged(QString,QString,QString)));

    connect(d->wallet, SIGNAL(walletClosed(int)), SLOT(slotWalletClosed(int)));
    connect(d->wallet, SIGNAL(folderListUpdated(QString)), SLOT(slotFolderListUpdated(QString)));
    connect(d->wallet, SIGNAL(folderUpdated(QString,QString)), SLOT(slotFolderUpdated(QString, QString)));
    connect(d->wallet, SIGNAL(applicationDisconnected(QString, QString)), SLOT(slotApplicationDisconnected(QString, QString)));

    // Verify that the wallet is still open
    if (d->handle != -1) {
        QDBusReply<bool> r = d->wallet->isOpen(d->handle);
        if (r.isValid() && !r) {
            d->handle = -1;
            d->name.clear();
        }
    }
}


Wallet::~Wallet() {
    if (d->handle != -1) {
        d->wallet->close(d->handle, false, appid());
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
    }
    delete d;
}


QStringList Wallet::walletList() {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    return wallet.wallets();
}


void Wallet::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kWarning() << "Pass a valid window to KWallet::Wallet::changePassword().";
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    wallet.changePassword(name, (qlonglong)w, appid());
}


bool Wallet::isEnabled() {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    return wallet.isEnabled(); // default is false
}


bool Wallet::isOpen(const QString& name) {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    return wallet.isOpen(name); // default is false
}


int Wallet::closeWallet(const QString& name, bool force) {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    QDBusReply<int> r = wallet.close(name, force);
    return r.isValid() ? r : -1;
}


int Wallet::deleteWallet(const QString& name) {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    QDBusReply<int> r = wallet.deleteWallet(name);
    return r.isValid() ? r : -1;
}


Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
    if( w == 0 )
        kWarning() << "Pass a valid window to KWallet::Wallet::openWallet().";
    if (ot == Asynchronous) {
        Wallet *wallet = new Wallet(-1, name);

        // place an asynchronous call
        QVariantList args;
        args << name << qlonglong(w) << appid();
        wallet->d->wallet->callWithCallback("open", args, wallet, SLOT(walletOpenResult(int)), SLOT(walletOpenError(const QDBusError&)));

        return wallet;
    }

    // avoid deadlock if the app has some popup open (#65978/#71048)
    while( QWidget* widget = qApp->activePopupWidget())
        widget->close();

    bool isPath = ot == Path;
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    QDBusReply<int> r = isPath ? wallet.openPath(name, (qlonglong)w, appid()) : wallet.open(name, (qlonglong)w, appid());
    if (r.isValid()) {
        int drc = r;
        if (drc != -1) {
            return new Wallet(drc, name);
        }
    }

    return 0;
}


bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
    org::kde::KWallet w("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    return w.disconnectApplication(wallet, app); // default is false
}


QStringList Wallet::users(const QString& name) {
    org::kde::KWallet wallet("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    return wallet.users(name); // default is QStringList()
}


int Wallet::sync() {
    if (d->handle == -1) {
        return -1;
    }

    d->wallet->sync(d->handle, appid());
    return 0;
}


int Wallet::lockWallet() {
    if (d->handle == -1) {
        return -1;
    }

    QDBusReply<int> r = d->wallet->close(d->handle, true, appid());
    d->handle = -1;
    d->folder.clear();
    d->name.clear();
    if (r.isValid()) {
        return r;
    }
    return -1;
}


const QString& Wallet::walletName() const {
    return d->name;
}


bool Wallet::isOpen() const {
    return d->handle != -1;
}


void Wallet::requestChangePassword(WId w) {
    if( w == 0 )
        kWarning() << "Pass a valid window to KWallet::Wallet::requestChangePassword().";
    if (d->handle == -1) {
        return;
    }

    d->wallet->changePassword(d->name, (qlonglong)w, appid());
}


void Wallet::slotWalletClosed(int handle) {
    if (d->handle == handle) {
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        emit walletClosed();
    }
}


QStringList Wallet::folderList() {
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = d->wallet->folderList(d->handle, appid());
    return r;
}


QStringList Wallet::entryList() {
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = d->wallet->entryList(d->handle, d->folder, appid());
    return r;
}


bool Wallet::hasFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->wallet->hasFolder(d->handle, f, appid());
    return r; // default is false
}


bool Wallet::createFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = d->wallet->createFolder(d->handle, f, appid());
        return r;
    }

    return true;				// folder already exists
}


bool Wallet::setFolder(const QString& f) {
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
}


bool Wallet::removeFolder(const QString& f) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->wallet->removeFolder(d->handle, f, appid());
    if (d->folder == f) {
        setFolder(QString());
    }

    return r;					// default is false
}


const QString& Wallet::currentFolder() const {
    return d->folder;
}


int Wallet::readEntry(const QString& key, QByteArray& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = d->wallet->readEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}


int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = d->wallet->readEntryList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        // convert <QString, QVariant> to <QString, QByteArray>
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toByteArray());
        }
    }

    return rc;
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->wallet->renameEntry(d->handle, d->folder, oldName, newName, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = d->wallet->readMap(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        QByteArray v = r;
        if (!v.isEmpty()) {
            QDataStream ds(&v, QIODevice::ReadOnly);
            ds >> value;
        }
    }

    return rc;
}


int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r =
        d->wallet->readMapList(d->handle, d->folder, key, appid());
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
}


int Wallet::readPassword(const QString& key, QString& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QString> r = d->wallet->readPassword(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}


int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = d->wallet->readPasswordList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toString());
        }
    }

    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->wallet->writeEntry(d->handle, d->folder, key, value, int(entryType), appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->wallet->writeEntry(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    QDBusReply<int> r = d->wallet->writeMap(d->handle, d->folder, key, mapData, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int Wallet::writePassword(const QString& key, const QString& value) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->wallet->writePassword(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


bool Wallet::hasEntry(const QString& key) {
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->wallet->hasEntry(d->handle, d->folder, key, appid());
    return r;					// default is false
}


int Wallet::removeEntry(const QString& key) {
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->wallet->removeEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


Wallet::EntryType Wallet::entryType(const QString& key) {
    int rc = 0;

    if (d->handle == -1) {
        return Wallet::Unknown;
    }

    QDBusReply<int> r = d->wallet->entryType(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return static_cast<EntryType>(rc);
}


void Wallet::slotServiceOwnerChanged(const QString& name,const QString& oldOwner,const QString& newOwner) {
    Q_UNUSED(oldOwner);
    if (d->handle >= 0 && newOwner.isEmpty() && name == "org.kde.kded") {
        slotWalletClosed(d->handle);
    }
}


void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder) {
    if (d->name == wallet) {
        emit folderUpdated(folder);
    }
}


void Wallet::slotFolderListUpdated(const QString& wallet) {
    if (d->name == wallet) {
        emit folderListUpdated();
    }
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application) {
    if (d->handle >= 0
        && d->name == wallet
        && application == QDBusConnection::sessionBus().baseService()) {
        slotWalletClosed(d->handle);
    }
}


void Wallet::walletOpenResult(int id) {
    if (d->handle != -1) {
        // This is BAD.
        return;
    }

    if (id > 0) {
        d->handle = id;
        emit walletOpened(true);
    } else if (id < 0) {
        emit walletOpened(false);
    } // id == 0 => wait
}

void Wallet::walletOpenError(const QDBusError& error)
{
    if (error.isValid()) {
        emit walletOpened(false);
    }
}

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    org::kde::KWallet w("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    QDBusReply<bool> r = w.folderDoesNotExist(wallet, folder);
    return r;
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    org::kde::KWallet w("org.kde.kded", "/modules/kwalletd", QDBusConnection::sessionBus());
    QDBusReply<bool> r = w.keyDoesNotExist(wallet, folder, key);
    return r;
}

void Wallet::virtual_hook(int, void*) {
    //BASE::virtual_hook( id, data );
}

#include "kwallet.moc"

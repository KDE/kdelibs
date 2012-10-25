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

#include "kwalletdefaultplugin.h"
#include "kwalletpluginloader.h"
#include "kwallet.h"
#include <ktoolinvocation.h>
#include <ksharedconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kwindowsystem.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <QDBusConnection>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QString>
#include "kwallet_interface.h"

typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)


namespace KWallet {

class KWalletDLauncher
{
public:
    KWalletDLauncher();
    ~KWalletDLauncher();

    org::kde::KWallet &getInterface();

    // this static variable is used below to switch between old KWallet
    // infrastructure and the new one which is built on top of the new
    // KSecretsService infrastructure. It's value can be changed via the 
    // the Wallet configuration module in System Settings
    bool m_useKSecretsService;
    WalletPlugin *m_plugin;
    KConfigGroup m_cgroup;
    WalletPluginLoader *m_pluginLoader;
};


K_GLOBAL_STATIC(KWalletDLauncher, walletLauncher)


static const char s_kwalletdServiceName[] = "org.kde.kwalletd";


class WalletDefaultPluginPrivate
{
public:
    WalletDefaultPluginPrivate( WalletDefaultPlugin* walletPlugin ) :
        m_walletPlugin( walletPlugin ),
        m_cgroup(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet"))
    {
        m_walletInterface = new org::kde::KWallet(QString::fromLatin1(s_kwalletdServiceName), "/modules/kwalletd", QDBusConnection::sessionBus());
    }
    void walletServiceUnregistered();

    org::kde::KWallet &getInterface();

    WalletDefaultPlugin *m_walletPlugin;
    org::kde::KWallet *m_walletInterface;
    KConfigGroup m_cgroup;
    int m_handle;
    int m_transactionId;
    QString m_name;
    QString m_folder;
    QPointer<QEventLoop> m_loop;
};

org::kde::KWallet &WalletDefaultPluginPrivate::getInterface()
{
    Q_ASSERT(m_walletInterface != 0);

    // check if kwalletd is already running
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName)))
    {
        // not running! check if it is enabled.
        bool walletEnabled = m_cgroup.readEntry("Enabled", true);
        if (walletEnabled) {
            // wallet is enabled! try launching it
            QString error;
            int ret = KToolInvocation::startServiceByDesktopPath("kwalletd.desktop", QStringList(), &error);
            if (ret > 0)
            {
                kError(285) << "Couldn't start kwalletd: " << error << endl;
            }

            if
                (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName))) {
                kDebug(285) << "The kwalletd service is still not registered";
            } else {
                kDebug(285) << "The kwalletd service has been registered";
            }
        } else {
            kError(285) << "The kwalletd service has been disabled";
        }
    }

    return *m_walletInterface;
}


static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
        qDBusRegisterMetaType<StringByteArrayMap>();
        registered = true;
    }
}

static QString appid()
{
    if (KGlobal::hasMainComponent()) {
        KComponentData cData = KGlobal::mainComponent();
        if (cData.isValid()) {
            const KAboutData* aboutData = cData.aboutData();
            if (aboutData) {
                return aboutData->programName();
            }
            return cData.componentName();
        }
    }
    return qApp->applicationName();
}


WalletDefaultPlugin::WalletDefaultPlugin() :
    d( new WalletDefaultPluginPrivate(this) )
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QString::fromLatin1(s_kwalletdServiceName), QDBusConnection::sessionBus(),
                                                        QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(walletServiceUnregistered()));

    connect(&d->getInterface(), SIGNAL(walletClosed(int)), SLOT(slotWalletClosed(int)));
    connect(&d->getInterface(), SIGNAL(folderListUpdated(QString)), SLOT(slotFolderListUpdated(QString)));
    connect(&d->getInterface(), SIGNAL(folderUpdated(QString,QString)), SLOT(slotFolderUpdated(QString,QString)));
    connect(&d->getInterface(), SIGNAL(applicationDisconnected(QString,QString)), SLOT(slotApplicationDisconnected(QString,QString)));

    // Verify that the wallet is still open
    if (d->m_handle != -1) {
        QDBusReply<bool> r = d->getInterface().isOpen(d->m_handle);
        if (r.isValid() && !r) {
            d->m_handle = -1;
            d->m_name.clear();
        }
    }
}

WalletDefaultPlugin::~WalletDefaultPlugin()
{
    if (d->m_handle != -1) {
        if (!walletLauncher.isDestroyed()) {
            d->getInterface().close(d->m_handle, false, appid());
        } else {
            kDebug(285) << "Problem with static destruction sequence."
                        "Destroy any static Wallet before the event-loop exits.";
        }
        d->m_handle = -1;
        d->m_folder.clear();
        d->m_name.clear();
    }
    delete d;
}

void WalletDefaultPlugin::slotWalletClosed(int handle) {
    if (d->m_handle == handle) {
        d->m_handle = -1;
        d->m_folder.clear();
        d->m_name.clear();
        emit walletClosed();
    }
}

void WalletDefaultPlugin::slotFolderUpdated(const QString& wallet, const QString& folder) {
    if (d->m_name == wallet) {
        emit folderUpdated(folder);
    }
}

void WalletDefaultPlugin::slotFolderListUpdated(const QString& wallet) {
    if (d->m_name == wallet) {
        emit folderListUpdated();
    }
}

void WalletDefaultPlugin::slotApplicationDisconnected(const QString& wallet, const QString& application) {
    if (d->m_handle >= 0
        && d->m_name == wallet
        && application == appid()) {
        slotWalletClosed(d->m_handle);
    }
}


QStringList WalletDefaultPlugin::walletList()
{
    QStringList result;
    QDBusReply<QStringList> r = d->getInterface().wallets();

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
    }
    else
        result = r;
    return result;
}


void WalletDefaultPlugin::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWalletDefaultPlugin::WalletDefaultPlugin::changePassword().";

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();
    d->getInterface().changePassword(name, (qlonglong)w, appid());
}


bool WalletDefaultPlugin::isEnabled() {
    QDBusReply<bool> r = d->getInterface().isEnabled();

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
    }
    else
        return r;
}


bool WalletDefaultPlugin::isOpen(const QString& name) {
    QDBusReply<bool> r = d->getInterface().isOpen(name);

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
    }
    else
        return r;
}

int WalletDefaultPlugin::closeWallet(const QString& name, bool force) {
    QDBusReply<int> r = d->getInterface().close(name, force);

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return -1;
    }
    else
        return r;
}


int WalletDefaultPlugin::deleteWallet(const QString& name) {
    QDBusReply<int> r = d->getInterface().deleteWallet(name);

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return -1;
    }
    else
        return r;
}

bool WalletDefaultPlugin::openWallet(const QString& name, WId w, OpenType ot) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWalletDefaultPlugin::WalletDefaultPlugin::openWallet().";

    // connect the daemon's opened signal to the slot filtering the
    // signals we need
    connect(&d->getInterface(), SIGNAL(walletAsyncOpened(int,int)),
            this, SLOT(walletAsyncOpened(int,int)));

    // Use an eventloop for synchronous calls
    QEventLoop loop;
    if (ot == Synchronous || ot == Path) {
        connect(this, SIGNAL(walletOpened(bool)), &loop, SLOT(quit()));
    }

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();

    // do the call
    QDBusReply<int> r;
    if (ot == Synchronous || ot == Asynchronous) {
        r = d->getInterface().openAsync(name, (qlonglong)w, appid(), true);
    } else if (ot == Path) {
        r = d->getInterface().openPathAsync(name, (qlonglong)w, appid(), true);
    } else {
        return false;
    }
    // error communicating with the daemon (maybe not running)
    if (!r.isValid()) {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    }
    d->m_transactionId = r.value();

    if (ot == Synchronous || ot == Path) {
        // check for an immediate error
        if (d->m_transactionId < 0) {
            return false;
        } else {
            // wait for the daemon's reply
            // store a pointer to the event loop so it can be quit in error case
            d->m_loop = &loop;
            loop.exec();
            if (d->m_handle < 0) {
                return false;
            }
        }
    } else if (ot == Asynchronous) {
        if (d->m_transactionId < 0) {
            QTimer::singleShot(0, this, SLOT(emitWalletAsyncOpenError()));
        }
    }

    return true;
}

bool WalletDefaultPlugin::disconnectApplication(const QString& wallet, const QString& app) {
    QDBusReply<bool> r = d->getInterface().disconnectApplication(wallet, app);

    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
    }
    else
        return r;
}


QStringList WalletDefaultPlugin::users(const QString& name) {
    QDBusReply<QStringList> r = d->getInterface().users(name);
    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return QStringList();
    }
    else
        return r;
}


int WalletDefaultPlugin::sync() {
    if (d->m_handle == -1) {
        return -1;
    }

    d->getInterface().sync(d->m_handle, appid());
    return 0;
}


int WalletDefaultPlugin::lockWallet() {
    if (d->m_handle == -1) {
        return -1;
    }

    QDBusReply<int> r = d->getInterface().close(d->m_handle, true, appid());
    d->m_handle = -1;
    d->m_folder.clear();
    d->m_name.clear();
    if (r.isValid()) {
        return r;
    }
    else {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return -1;
    }
}


const QString& WalletDefaultPlugin::walletName() const {
    return d->m_name;
}


bool WalletDefaultPlugin::isOpen() const {
    return d->m_handle != -1;
}


void WalletDefaultPlugin::requestChangePassword(WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWalletDefaultPlugin::WalletDefaultPlugin::requestChangePassword().";

    if (d->m_handle == -1) {
        return;
    }

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();

    d->getInterface().changePassword(d->m_name, (qlonglong)w, appid());
}

QStringList WalletDefaultPlugin::folderList() {
    if (d->m_handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = d->getInterface().folderList(d->m_handle, appid());
    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return QStringList();
    }
    else
        return r;
}


QStringList WalletDefaultPlugin::entryList() {
    if (d->m_handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = d->getInterface().entryList(d->m_handle, d->m_folder, appid());
    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return QStringList();
    }
    else
        return r;
}


bool WalletDefaultPlugin::hasFolder(const QString& f) {
    if (d->m_handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->getInterface().hasFolder(d->m_handle, f, appid());
    if (!r.isValid())
    {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
    }
    else
        return r;
}


bool WalletDefaultPlugin::createFolder(const QString& f) {
    if (d->m_handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = d->getInterface().createFolder(d->m_handle, f, appid());

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }
    return false;
}


bool WalletDefaultPlugin::setFolder(const QString& f) {
    bool rc = false;
    if (d->m_handle == -1) {
        return rc;
    }

    // Don't do this - the folder could have disappeared?
#if 0
    if (f == d->m_folder) {
        return true;
    }
#endif

    if (hasFolder(f)) {
        d->m_folder = f;
        rc = true;
    }

    return rc;
}

bool WalletDefaultPlugin::removeFolder(const QString& f) {
    if (d->m_handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->getInterface().removeFolder(d->m_handle, f, appid());
    if (d->m_folder == f) {
        setFolder(QString());
    }

    if (!r.isValid())
    {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    }
    else
        return r;
}


const QString& WalletDefaultPlugin::currentFolder() const {
    return d->m_folder;
}

int WalletDefaultPlugin::readEntry(const QString& key, QByteArray& value) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = d->getInterface().readEntry(d->m_handle, d->m_folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}

int WalletDefaultPlugin::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {
    int rc =-1;
    registerTypes();

    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = d->getInterface().readEntryList(d->m_handle, d->m_folder, key, appid());
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


int WalletDefaultPlugin::renameEntry(const QString& oldName, const QString& newName) {
    int rc =-1;

    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->getInterface().renameEntry(d->m_handle, d->m_folder, oldName, newName, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int WalletDefaultPlugin::readMap(const QString& key, QMap<QString,QString>& value) {
    int rc =-1;

    registerTypes();

    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = d->getInterface().readMap(d->m_handle, d->m_folder, key, appid());
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

int WalletDefaultPlugin::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    int rc =-1;
    registerTypes();

    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r =
        d->getInterface().readMapList(d->m_handle, d->m_folder, key, appid());
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


int WalletDefaultPlugin::readPassword(const QString& key, QString& value) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QString> r = d->getInterface().readPassword(d->m_handle, d->m_folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}

int WalletDefaultPlugin::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    int rc =-1;
    registerTypes();

    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = d->getInterface().readPasswordList(d->m_handle, d->m_folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
            value.insert(it.key(), it.value().toString());
        }
    }

    return rc;
}


int WalletDefaultPlugin::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->getInterface().writeEntry(d->m_handle, d->m_folder, key, value, int(entryType), appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int WalletDefaultPlugin::writeEntry(const QString& key, const QByteArray& value) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->getInterface().writeEntry(d->m_handle, d->m_folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int WalletDefaultPlugin::writeMap(const QString& key, const QMap<QString,QString>& value) {
    int rc =-1;
    registerTypes();

    if (d->m_handle == -1) {
        return rc;
    }

    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    QDBusReply<int> r = d->getInterface().writeMap(d->m_handle, d->m_folder, key, mapData, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


int WalletDefaultPlugin::writePassword(const QString& key, const QString& value) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->getInterface().writePassword(d->m_handle, d->m_folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


bool WalletDefaultPlugin::hasEntry(const QString& key) {
    int rc =-1;
    if (d->m_handle == -1) {
        return false;
    }

    QDBusReply<bool> r = d->getInterface().hasEntry(d->m_handle, d->m_folder, key, appid());
    if (!r.isValid())
    {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    }
    else
        return r;
}


int WalletDefaultPlugin::removeEntry(const QString& key) {
    int rc =-1;
    if (d->m_handle == -1) {
        return rc;
    }

    QDBusReply<int> r = d->getInterface().removeEntry(d->m_handle, d->m_folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}


WalletDefaultPlugin::EntryType WalletDefaultPlugin::entryType(const QString& key) {
    int rc =-1;
    if (d->m_handle == -1) {
        return WalletDefaultPlugin::Unknown;
    }

    QDBusReply<int> r = d->getInterface().entryType(d->m_handle, d->m_folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }
    return static_cast<EntryType>(rc);
}


void WalletDefaultPluginPrivate::walletServiceUnregistered()
{
    if (m_loop) {
        m_loop->quit();
    }

    if (m_handle >= 0) {
        m_walletPlugin->slotWalletClosed(m_handle);
    }
}

void WalletDefaultPlugin::walletAsyncOpened(int tId, int handle) {
    // ignore responses to calls other than ours
    if (d->m_transactionId != tId || d->m_handle != -1) {
        return;
    }

    // disconnect the async signal
    disconnect(this, SLOT(walletAsyncOpened(int,int)));

    d->m_handle = handle;
    emit walletOpened(handle > 0);
}

void WalletDefaultPlugin::emitWalletAsyncOpenError() {
    emit walletOpened(false);
}

void WalletDefaultPlugin::emitWalletOpened() {
  emit walletOpened(true);
}

bool WalletDefaultPlugin::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    bool result = false;
    QDBusReply<bool> r = d->getInterface().folderDoesNotExist(wallet, folder);
    if (!r.isValid())
    {
        kDebug(285) << "Invalid DBus reply: " << r.error();
    }
    else
        result = r;
    return result;
}


bool WalletDefaultPlugin::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    QDBusReply<bool> r = d->getInterface().keyDoesNotExist(wallet, folder, key);
    if (!r.isValid())
    {
        kDebug(285) << "Invalid DBus reply: " << r.error();
        return false;
    }
    else
        return r;
}

KWalletDLauncher::KWalletDLauncher()
    : m_plugin(0),
    m_cgroup(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet"))
{
    // TODO: plugin loading should occur here
    m_useKSecretsService = m_cgroup.readEntry("UseKSecretsService", false);
    if (m_useKSecretsService) {
         m_plugin = m_pluginLoader->loadKSecrets();
    }
    else {
        m_plugin = m_pluginLoader->loadKWallet();
    }
}

KWalletDLauncher::~KWalletDLauncher()
{
    delete m_plugin;
}


} // namespace

#include "kwalletdefaultplugin.moc"

/* This file is part of the KDE project
 *
 * Copyright (C) 2002-2004 George Staikos <staikos@kde.org>
 * Copyright (C) 2008 Michael Leupold <lemma@confuego.org>
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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
#include "../ksecretsservice/ksecretsservicecollection.h"
#include "../ksecretsservicecollectionjobs.h"
#include "../ksecretsservice/ksecretsserviceitem.h"
#include "../ksecretsservice/ksecretsserviceitemjobs.h"
#include <ksharedconfig.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <QtDBus/QtDBus>
#include <ktoolinvocation.h>

#include <assert.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kconfiggroup.h>
#include <kwindowsystem.h>

#include "kwallet_interface.h"

using namespace KWallet;
using namespace KSecretsService;



typedef QMap<QString, StringStringMap> StringToStringStringMapMap;
Q_DECLARE_METATYPE(StringToStringStringMapMap)
typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

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
    org::kde::KWallet *m_wallet;
    KConfigGroup m_cgroup;
};

K_GLOBAL_STATIC(KWalletDLauncher, walletLauncher)

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
    // NOTE: This method stays unchanged for KSecretsService
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

const QString Wallet::NetworkWallet() {
    // NOTE: This method stays unchanged for KSecretsService
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));

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
    WalletPrivate(Wallet *wallet, int h, const QString &n)
     : q(wallet), name(n), handle(h),
     secretsCollection(0)
    {}

    void walletServiceUnregistered();

    template <typename T> 
    int writeEntry( const QString& key, const T &value, Wallet::EntryType entryType ) {
        int rc = -1;
        Secret secret;
        secret.setValue( QVariant::fromValue<T>(value) );

        StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = folder;
        attrs[KSS_ATTR_WALLETTYPE] = QString("%1").arg((int)entryType);
        CreateCollectionItemJob *createItemJob = secretsCollection->createItem( key, attrs, secret );

        if ( !createItemJob->exec() ) {
            kDebug(285) << "Cannot execute CreateCollectionItemJob : " << createItemJob->errorString();
        }
        rc = createItemJob->error();
        return rc;
    }
    
    SecretItem *findItem( const QString& key ) const;
    template <typename T> int readEntry( const QString& key, T& value ) const;
    bool readSecret( const QString& key, Secret& value ) const;
    
    template <typename V>
    int forEachItemThatMatches( const QString &key, V verb ) {
        int rc = -1;
        StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = folder;
        SearchCollectionItemsJob *searchItemsJob = secretsCollection->searchItems(attrs);
        if ( searchItemsJob->exec() ) {
            QRegExp re(key, Qt::CaseSensitive, QRegExp::Wildcard);
            foreach( SearchCollectionItemsJob::Item item , searchItemsJob->items() ) {
                ReadItemPropertyJob *readLabelJob = item->label();
                if ( readLabelJob->exec() ) {
                    QString label = readLabelJob->propertyValue().toString();
                    if ( re.exactMatch( label ) ) {
                        if ( verb( this, label, item.data() ) ) {
                            rc = 0; // one successfull iteration already produced results, so success return
                        }
                    }
                }
                else {
                    kDebug(285) << "Cannot execute ReadItemPropertyJob " << readLabelJob->errorString();
                }
            }
        }
        else {
            kDebug(285) << "Cannot execute SearchCollectionItemsJob " << searchItemsJob->errorString();
        }
        return rc;
    }
    
    struct InsertIntoEntryList;
    struct InsertIntoMapList;
    struct InsertIntoPasswordList;

    Wallet *q;
    QString name;
    QString folder;
    int handle;
    int transactionId;
    QPointer<QEventLoop> loop;

    Collection *secretsCollection;
};

static const char s_kwalletdServiceName[] = "org.kde.kwalletd";

Wallet::Wallet(int handle, const QString& name)
    : QObject(0L), d(new WalletPrivate(this, handle, name))
{
    if (walletLauncher->m_useKSecretsService) {
        // see openWallet for initialization code; this constructor does not have any code
    }
    else {
        QDBusServiceWatcher *watcher = new QDBusServiceWatcher(QString::fromLatin1(s_kwalletdServiceName), QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForUnregistration, this);
        connect(watcher, SIGNAL(serviceUnregistered(QString)),
                this, SLOT(walletServiceUnregistered()));

        connect(&walletLauncher->getInterface(), SIGNAL(walletClosed(int)), SLOT(slotWalletClosed(int)));
        connect(&walletLauncher->getInterface(), SIGNAL(folderListUpdated(QString)), SLOT(slotFolderListUpdated(QString)));
        connect(&walletLauncher->getInterface(), SIGNAL(folderUpdated(QString,QString)), SLOT(slotFolderUpdated(QString, QString)));
        connect(&walletLauncher->getInterface(), SIGNAL(applicationDisconnected(QString, QString)), SLOT(slotApplicationDisconnected(QString, QString)));

        // Verify that the wallet is still open
        if (d->handle != -1) {
            QDBusReply<bool> r = walletLauncher->getInterface().isOpen(d->handle);
            if (r.isValid() && !r) {
                d->handle = -1;
                d->name.clear();
            }
        }
    }
}


Wallet::~Wallet() {
    if (walletLauncher->m_useKSecretsService) {
        d->folder.clear();
        d->name.clear();
        delete d->secretsCollection;
    }
    else {
        if (d->handle != -1) {
            if (!walletLauncher.isDestroyed()) {
                walletLauncher->getInterface().close(d->handle, false, appid());
            } else {
                kDebug(285) << "Problem with static destruction sequence."
                            "Destroy any static Wallet before the event-loop exits.";
            }
            d->handle = -1;
            d->folder.clear();
            d->name.clear();
        }
    }
    delete d;
}


QStringList Wallet::walletList() {
    QStringList result;
    if (walletLauncher->m_useKSecretsService) {
        ListCollectionsJob *listJob = Collection::listCollections();
        if ( listJob->exec() ) {
            result = listJob->collections();
        }
        else {
            kDebug(285) << "Cannot execute ListCollectionsJob: " << listJob->errorString();
        }
    }
    else {
        QDBusReply<QStringList> r = walletLauncher->getInterface().wallets();

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
        }
        else
            result = r;
    }
    return result;
}


void Wallet::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::changePassword().";

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();
    if (walletLauncher->m_useKSecretsService) {
        Collection *coll = Collection::findCollection( name );
        ChangeCollectionPasswordJob* changePwdJob = coll->changePassword();
        if ( !changePwdJob->exec() ) {
            kDebug(285) << "Cannot execute change password job: " << changePwdJob->errorString();
        }
        coll->deleteLater();
    }
    else {
        walletLauncher->getInterface().changePassword(name, (qlonglong)w, appid());
    }
}


bool Wallet::isEnabled() {
    if (walletLauncher->m_useKSecretsService) {
        return walletLauncher->m_cgroup.readEntry("Enabled", true);
    }
    else {
        QDBusReply<bool> r = walletLauncher->getInterface().isEnabled();

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }
}


bool Wallet::isOpen(const QString& name) {
    if (walletLauncher->m_useKSecretsService) {
        Collection *coll = Collection::findCollection( name, Collection::OpenOnly );
        ReadCollectionPropertyJob *readLocked = coll->isLocked();
        if ( readLocked->exec() ) {
            return !readLocked->propertyValue().toBool();
        }
        else {
            kDebug() << "ReadLocked job failed";
            return false;
        }
    }
    else {
        QDBusReply<bool> r = walletLauncher->getInterface().isOpen(name);

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }
}

int Wallet::closeWallet(const QString& name, bool force) {
    if (walletLauncher->m_useKSecretsService) {
        kDebug(285) << "Wallet::closeWallet NOOP";
        return 0;
    }
    else {
        QDBusReply<int> r = walletLauncher->getInterface().close(name, force);

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return -1;
        }
        else
            return r;
    }
}


int Wallet::deleteWallet(const QString& name) {
    if (walletLauncher->m_useKSecretsService) {
        Collection *coll = Collection::findCollection(name, Collection::OpenOnly);
        KJob *deleteJob = coll->deleteCollection();
        if (!deleteJob->exec()) {
            kDebug(285) << "Cannot execute delete job " << deleteJob->errorString();
        }
        return deleteJob->error();
    }
    else {
        QDBusReply<int> r = walletLauncher->getInterface().deleteWallet(name);

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return -1;
        }
        else
            return r;
    }
}

Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::openWallet().";

    if (walletLauncher->m_useKSecretsService) {
        Wallet *wallet = new Wallet(-1, name);
        // FIXME: should we specify CreateCollection or OpenOnly here?
        wallet->d->secretsCollection = Collection::findCollection(name, Collection::CreateCollection, QVariantMap(), w);
        if ( ot == Synchronous ) {
           kDebug() << "WARNING openWallet OpenType=Synchronous requested";
           // TODO: not sure what to do with in this case; however, all other KSecretsService API methods are already
           // async and will perform sync inside this API because of it's design
        }
        return wallet;
    }
    else {
        Wallet *wallet = new Wallet(-1, name);

        // connect the daemon's opened signal to the slot filtering the
        // signals we need
        connect(&walletLauncher->getInterface(), SIGNAL(walletAsyncOpened(int, int)),
                wallet, SLOT(walletAsyncOpened(int, int)));

        // Use an eventloop for synchronous calls
        QEventLoop loop;
        if (ot == Synchronous || ot == Path) {
            connect(wallet, SIGNAL(walletOpened(bool)), &loop, SLOT(quit()));
        }

        // Make sure the password prompt window will be visible and activated
        KWindowSystem::allowExternalProcessWindowActivation();

        // do the call
        QDBusReply<int> r;
        if (ot == Synchronous || ot == Asynchronous) {
            r = walletLauncher->getInterface().openAsync(name, (qlonglong)w, appid(), true);
        } else if (ot == Path) {
            r = walletLauncher->getInterface().openPathAsync(name, (qlonglong)w, appid(), true);
        } else {
            delete wallet;
            return 0;
        }
        // error communicating with the daemon (maybe not running)
        if (!r.isValid()) {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            delete wallet;
            return 0;
        }
        wallet->d->transactionId = r.value();

        if (ot == Synchronous || ot == Path) {
            // check for an immediate error
            if (wallet->d->transactionId < 0) {
                delete wallet;
                wallet = 0;
            } else {
                // wait for the daemon's reply
                // store a pointer to the event loop so it can be quit in error case
                wallet->d->loop = &loop;
                loop.exec();
                if (wallet->d->handle < 0) {
                    delete wallet;
                    return 0;
                }
            }
        } else if (ot == Asynchronous) {
            if (wallet->d->transactionId < 0) {
                QTimer::singleShot(0, wallet, SLOT(emitWalletAsyncOpenError()));
                // client code is responsible for deleting the wallet
            }
        }

        return wallet;
    }
}


bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
    if (walletLauncher->m_useKSecretsService) {
        kDebug() << "Wallet::disconnectApplication NOOP";
        return true;
    }
    else {
        QDBusReply<bool> r = walletLauncher->getInterface().disconnectApplication(wallet, app);

        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }
}


QStringList Wallet::users(const QString& name) {
    if (walletLauncher->m_useKSecretsService) {
        kDebug() << "KSecretsService does not handle users list";
        return QStringList();
    }
    else {
        QDBusReply<QStringList> r = walletLauncher->getInterface().users(name);
        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return QStringList();
        }
        else
            return r;
    }
}


int Wallet::sync() {
    if (walletLauncher->m_useKSecretsService) {
        // NOOP with KSecretsService
    }
    else {
        if (d->handle == -1) {
            return -1;
        }

        walletLauncher->getInterface().sync(d->handle, appid());
    }
    return 0;
}


int Wallet::lockWallet() {
    if (walletLauncher->m_useKSecretsService) {
        CollectionLockJob *lockJob = d->secretsCollection->lock();
        if (lockJob->exec()) {
            d->folder.clear();
            d->name.clear();
        }
        else {
            kDebug(285) << "Cannot execute CollectionLockJob : " << lockJob->errorString();
            return -1;
        }
        return lockJob->error();
    }
    else {
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
        else {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return -1;
        }
    }
}


const QString& Wallet::walletName() const {
    return d->name;
}


bool Wallet::isOpen() const {
    if (walletLauncher->m_useKSecretsService) {
        return !d->secretsCollection->isLocked();
    }
    else {
        return d->handle != -1;
    }
}


void Wallet::requestChangePassword(WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";

    if (walletLauncher->m_useKSecretsService) {
        ChangeCollectionPasswordJob *changePwdJob = d->secretsCollection->changePassword();
        if (!changePwdJob->exec()) {
            kDebug(285) << "Cannot execute ChangeCollectionPasswordJob : " << changePwdJob->errorString();
        }
    }
    else {
        if (d->handle == -1) {
            return;
        }

        // Make sure the password prompt window will be visible and activated
        KWindowSystem::allowExternalProcessWindowActivation();

        walletLauncher->getInterface().changePassword(d->name, (qlonglong)w, appid());
    }
}


void Wallet::slotWalletClosed(int handle) {
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
        if (d->handle == handle) {
            d->handle = -1;
            d->folder.clear();
            d->name.clear();
            emit walletClosed();
        }
    }
}


QStringList Wallet::folderList() {
    if (walletLauncher->m_useKSecretsService) {
        QStringList result;
        ReadCollectionItemsJob *searchJob = d->secretsCollection->items();
        if (searchJob->exec()) {
            ReadCollectionItemsJob::ItemList itemList = searchJob->items();
            foreach( const ReadCollectionItemsJob::Item &item, itemList ) {
                ReadItemPropertyJob *readAttrsJob = item->attributes();
                if (readAttrsJob->exec()) {
                    StringStringMap attrs = readAttrsJob->propertyValue().value<StringStringMap>();
                    const QString folder = attrs[KSS_ATTR_ENTRYFOLDER];
                    if (!folder.isEmpty() && !result.contains(folder)) {
                        result.append(folder);
                    }
                }
                else {
                    kDebug(285) << "Cannot read item attributes : " << readAttrsJob->errorString();
                }
            }
        }
        else {
            kDebug(285) << "Cannot execute ReadCollectionItemsJob : " << searchJob->errorString();
        }
        return result;
    }
    else {
        if (d->handle == -1) {
            return QStringList();
        }

        QDBusReply<QStringList> r = walletLauncher->getInterface().folderList(d->handle, appid());
        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return QStringList();
        }
        else
            return r;
    }
}


QStringList Wallet::entryList() {
    if (walletLauncher->m_useKSecretsService) {
        QStringList result;
        StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = d->folder;
        SearchCollectionItemsJob *readItemsJob = d->secretsCollection->searchItems( attrs );
        if ( readItemsJob->exec() ) {
            foreach( SearchCollectionItemsJob::Item item, readItemsJob->items() ) {
                ReadItemPropertyJob *readLabelJob = item->label();
                if ( readLabelJob->exec() ) {
                    result.append( readLabelJob->propertyValue().toString() );
                }
                else {
                    kDebug(285) << "Cannot execute readLabelJob" << readItemsJob->errorString();
                }
            }
        }
        else {
            kDebug(285) << "Cannot execute readItemsJob" << readItemsJob->errorString();
        }
        return result;
    }
    else {
        if (d->handle == -1) {
            return QStringList();
        }

        QDBusReply<QStringList> r = walletLauncher->getInterface().entryList(d->handle, d->folder, appid());
        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return QStringList();
        }
        else
            return r;
    }
}


bool Wallet::hasFolder(const QString& f) {
    if (walletLauncher->m_useKSecretsService) {
        // FIXME: well, this is not the best implementation, but it's done quickly :)
        // the best way would be to searchItems with the attribute label having the value f
        // doing that would reduce DBus traffic. But KWallet API wille not last.
        QStringList folders = folderList();
        return folders.contains(f);
    }
    else {
        if (d->handle == -1) {
            return false;
        }

        QDBusReply<bool> r = walletLauncher->getInterface().hasFolder(d->handle, f, appid());
        if (!r.isValid())
        {
                kDebug(285) << "Invalid DBus reply: " << r.error();
                return false;
        }
        else
            return r;
    }
}


bool Wallet::createFolder(const QString& f) {
    if (walletLauncher->m_useKSecretsService) {
        kDebug(285) << "Wallet::createFolder NOOP";
        return true;
    }
    else {
        if (d->handle == -1) {
            return false;
        }

        if (!hasFolder(f)) {
            QDBusReply<bool> r = walletLauncher->getInterface().createFolder(d->handle, f, appid());

            if (!r.isValid())
            {
                    kDebug(285) << "Invalid DBus reply: " << r.error();
                    return false;
            }
            else
                return r;
        }

        return true;				// folder already exists
    }
}


bool Wallet::setFolder(const QString& f) {
    bool rc = false;

    if (walletLauncher->m_useKSecretsService) {
        if (hasFolder(f)) {
            d->folder = f;
            rc = true;
        }
    }
    else {
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
    }

    return rc;
}


bool Wallet::removeFolder(const QString& f) {
    if (walletLauncher->m_useKSecretsService) {
        bool result = false;
        // search for all items having the folder f then delete them
        StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = f;
        SearchCollectionItemsJob *searchJob = d->secretsCollection->searchItems(attrs);
        if (searchJob->exec()) {
            SearchCollectionItemsJob::ItemList itemList = searchJob->items();
            if ( !itemList.isEmpty() ) {
                result = true;
                foreach( const SearchCollectionItemsJob::Item &item, itemList ) {
                    SecretItemDeleteJob *deleteJob = item->deleteItem();
                    if (!deleteJob->exec()) {
                        kDebug(285) << "Cannot delete item : " << deleteJob->errorString();
                        result = false;
                    }
                    result &= true;
                }
            }
        }
        else {
            kDebug(285) << "Cannot execute SearchCollectionItemsJob : " << searchJob->errorString();
        }
        return result;
    }
    else {
        if (d->handle == -1) {
            return false;
        }

        QDBusReply<bool> r = walletLauncher->getInterface().removeFolder(d->handle, f, appid());
        if (d->folder == f) {
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
}


const QString& Wallet::currentFolder() const {
    return d->folder;
}

SecretItem *Wallet::WalletPrivate::findItem( const QString& key ) const
{
    SecretItem *result =0;
    StringStringMap attrs;
    attrs[KSS_ATTR_ENTRYFOLDER] = folder;
    attrs["Label"] = key;
    SearchCollectionItemsJob *searchJob = secretsCollection->searchItems(attrs);
    if (searchJob->exec()) {
        SearchCollectionItemsJob::ItemList itemList = searchJob->items();
        if ( !itemList.isEmpty() ) {
            result = itemList.first().data();
        }
        else {
            kDebug(285) << "entry named " << key << " not found in folder " << folder;
        }
    }
    else {
        kDebug(285) << "Cannot exec SearchCollectionItemsJob : " << searchJob->errorString();
    }

    return result;
}

template <typename T>
int Wallet::WalletPrivate::readEntry(const QString& key, T& value) const
{
    int rc = -1;
    SecretItem *item = findItem(key);
    GetSecretItemSecretJob *readJob = item->getSecret();
    if ( readJob->exec() ) {
        Secret theSecret = readJob->secret();
        kDebug(285) << "Secret contentType is " << theSecret.contentType();
        value = theSecret.value().value<T>();
        rc = 0;
    }
    else {
        kDebug(285) << "Cannot exec GetSecretItemSecretJob : " << readJob->errorString();
    }
    return rc;
}

bool Wallet::WalletPrivate::readSecret(const QString& key, Secret& value) const
{
    bool result = false;
    SecretItem *item = findItem(key);
    GetSecretItemSecretJob *readJob = item->getSecret();
    if ( readJob->exec() ) {
        value = readJob->secret();
        result = true;
    }
    else {
        kDebug(285) << "Cannot exec GetSecretItemSecretJob : " << readJob->errorString();
    }
    return result;
}

int Wallet::readEntry(const QString& key, QByteArray& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        return d->readEntry<QByteArray>(key, value);
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<QByteArray> r = walletLauncher->getInterface().readEntry(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            value = r;
            rc = 0;
        }
    }

    return rc;
}

struct Wallet::WalletPrivate::InsertIntoEntryList {
    InsertIntoEntryList( QMap< QString, QByteArray> &value ) : _value( value ) {}
    bool operator() ( Wallet::WalletPrivate*, const QString& label, SecretItem* item ) {
        bool result = false;
        GetSecretItemSecretJob *readSecretJob = item->getSecret();
        if ( readSecretJob->exec() ) {
            _value.insert( label, readSecretJob->secret().value().toByteArray() );
            result = true;
        }
        else {
            kDebug(285) << "Cannot execute GetSecretItemSecretJob " << readSecretJob->errorString();
        }
        return result;
    }
    QMap< QString, QByteArray > _value;
};

int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {

    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->forEachItemThatMatches( key, WalletPrivate::InsertIntoEntryList( value ) );
    }
    else {
        registerTypes();
        
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
    }

    return rc;
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        SecretItem *item = d->findItem(oldName);
        if (item) {
            WriteItemPropertyJob *writeJob = item->setLabel(newName);
            if (!writeJob->exec()) {
                kDebug(285) << "Cannot exec WriteItemPropertyJob : " << writeJob->errorString();
            }
            rc = writeJob->error();
        }
        else {
            kDebug(285) << "Cannot locate item " << oldName << " in folder " << d->folder;
        }
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<int> r = walletLauncher->getInterface().renameEntry(d->handle, d->folder, oldName, newName, appid());
        if (r.isValid()) {
            rc = r;
        }
    }

    return rc;
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->readEntry< QMap<QString,QString> >(key, value);
    }
    else {
        registerTypes();

        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<QByteArray> r = walletLauncher->getInterface().readMap(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            rc = 0;
            QByteArray v = r;
            if (!v.isEmpty()) {
                QDataStream ds(&v, QIODevice::ReadOnly);
                ds >> value;
            }
        }
    }

    return rc;
}

struct Wallet::WalletPrivate::InsertIntoMapList {
    InsertIntoMapList( QMap< QString, QMap< QString, QString > > &value ) : _value( value ) {}
    bool operator() ( Wallet::WalletPrivate* d, const QString& label, SecretItem* ) {
        bool result = false;
        QMap<QString, QString> map;
        if ( d->readEntry< QMap< QString, QString> >(label, map) ) {
            _value.insert( label, map );
            result = true;
        }
        return result;
    }
    QMap< QString, QMap< QString, QString> > &_value;
};

int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->forEachItemThatMatches( key, WalletPrivate::InsertIntoMapList( value ) );
    }
    else {
        registerTypes();

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
    }

    return rc;
}


int Wallet::readPassword(const QString& key, QString& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->readEntry<QString>(key, value);
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<QString> r = walletLauncher->getInterface().readPassword(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            value = r;
            rc = 0;
        }
    }

    return rc;
}

struct Wallet::WalletPrivate::InsertIntoPasswordList {
    InsertIntoPasswordList( QMap< QString, QString> &value ) : _value( value ) {}
    bool operator() ( Wallet::WalletPrivate* d, const QString& label, SecretItem* ) {
        bool result = false;
        QString pwd;
        if ( d->readEntry<QString>( label, pwd ) == 0 ) {
            _value.insert( label, pwd );
            result = true;
        }
        return result;
    }
    QMap< QString, QString > &_value;
};

int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->forEachItemThatMatches( key, WalletPrivate::InsertIntoPasswordList( value ) );
    }
    else {
        registerTypes();

        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<QVariantMap> r = walletLauncher->getInterface().readPasswordList(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            rc = 0;
            const QVariantMap val = r.value();
            for( QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it ) {
                value.insert(it.key(), it.value().toString());
            }
        }
    }
    
    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->writeEntry( key, value, entryType );
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<int> r = walletLauncher->getInterface().writeEntry(d->handle, d->folder, key, value, int(entryType), appid());
        if (r.isValid()) {
            rc = r;
        }
    }

    return rc;
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = writeEntry( key, value, Stream );
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<int> r = walletLauncher->getInterface().writeEntry(d->handle, d->folder, key, value, appid());
        if (r.isValid()) {
            rc = r;
        }
    }

    return rc;
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        d->writeEntry( key, value, Map );
    }
    else {
        registerTypes();
        
        if (d->handle == -1) {
            return rc;
        }

        QByteArray mapData;
        QDataStream ds(&mapData, QIODevice::WriteOnly);
        ds << value;
        QDBusReply<int> r = walletLauncher->getInterface().writeMap(d->handle, d->folder, key, mapData, appid());
        if (r.isValid()) {
            rc = r;
        }
    }

    return rc;
}


int Wallet::writePassword(const QString& key, const QString& value) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        rc = d->writeEntry( key, value, Password );
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<int> r = walletLauncher->getInterface().writePassword(d->handle, d->folder, key, value, appid());
        if (r.isValid()) {
            rc = r;
        }
    }
    
    return rc;
}


bool Wallet::hasEntry(const QString& key) {
    if (walletLauncher->m_useKSecretsService) {
        SecretItem *item = d->findItem( key );
        return item != 0;
    }
    else {
        if (d->handle == -1) {
            return false;
        }

        QDBusReply<bool> r = walletLauncher->getInterface().hasEntry(d->handle, d->folder, key, appid());
        if (!r.isValid())
        {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
        }
        else
            return r;
    }
}


int Wallet::removeEntry(const QString& key) {
    int rc = -1;

    if (walletLauncher->m_useKSecretsService) {
        SecretItem *item = d->findItem( key );
        if ( item ) {
            SecretItemDeleteJob *deleteJob = item->deleteItem();
            if ( !deleteJob->exec() ) {
                kDebug(285) << "Cannot execute SecretItemDeleteJob " << deleteJob->errorString();
            }
            rc = deleteJob->error();
        }
    }
    else {
        if (d->handle == -1) {
            return rc;
        }

        QDBusReply<int> r = walletLauncher->getInterface().removeEntry(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            rc = r;
        }
    }

    return rc;
}


Wallet::EntryType Wallet::entryType(const QString& key) {
    int rc = 0;

    if (walletLauncher->m_useKSecretsService) {
        SecretItem *item = d->findItem( key );
        if ( item ) {
            ReadItemPropertyJob *readAttrsJob = item->attributes();
            if ( readAttrsJob->exec() ) {
                StringStringMap attrs = readAttrsJob->propertyValue().value<StringStringMap>();
                if ( attrs.contains( KSS_ATTR_WALLETTYPE ) ) {
                    QString entryType = attrs[KSS_ATTR_WALLETTYPE];
                    bool ok = false;
                    rc = entryType.toInt( &ok );
                    if ( !ok ) {
                        rc = -1;
                    }
                    else {
                        kDebug(285) << KSS_ATTR_WALLETTYPE << " attribute holds non int value " << attrs[KSS_ATTR_WALLETTYPE];
                    }
                }
            }
            else {
                kDebug(285) << "Cannot execute GetSecretItemSecretJob " << readAttrsJob->errorString();
            }
        }
        return Unknown;
    }
    else {
        if (d->handle == -1) {
            return Wallet::Unknown;
        }

        QDBusReply<int> r = walletLauncher->getInterface().entryType(d->handle, d->folder, key, appid());
        if (r.isValid()) {
            rc = r;
        }

        return static_cast<EntryType>(rc);
    }
}


void Wallet::WalletPrivate::walletServiceUnregistered()
{
    if (loop) {
        loop->quit();
    }

    if (handle >= 0) {
        q->slotWalletClosed(handle);
    }
}

void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder) {
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
        if (d->name == wallet) {
            emit folderUpdated(folder);
        }
    }
}


void Wallet::slotFolderListUpdated(const QString& wallet) {
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
        if (d->name == wallet) {
            emit folderListUpdated();
        }
    }
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application) {
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
        if (d->handle >= 0
            && d->name == wallet
            && application == appid()) {
            slotWalletClosed(d->handle);
        }
    }
}

void Wallet::walletAsyncOpened(int tId, int handle) {
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
        // ignore responses to calls other than ours
        if (d->transactionId != tId || d->handle != -1) {
            return;
        }

        // disconnect the async signal
        disconnect(this, SLOT(walletAsyncOpened(int, int)));

        d->handle = handle;
        emit walletOpened(handle > 0);
    }
}

void Wallet::emitWalletAsyncOpenError() {
    emit walletOpened(false);
}

void Wallet::emitWalletOpened() {
  emit walletOpened(true);
}

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    if (walletLauncher->m_useKSecretsService) {
        kDebug(285) << "WARNING: changing semantics of folderDoesNotExist with KSS: will prompt for the password";
        Wallet *w = openWallet( wallet, 0, Synchronous );
        if ( w ) {
            return !w->hasFolder( folder );
        }
        else {
            return true;
        }
    }
    else {
        QDBusReply<bool> r = walletLauncher->getInterface().folderDoesNotExist(wallet, folder);
        if (!r.isValid())
        {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
        }
        else
            return r;
    }
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    if (walletLauncher->m_useKSecretsService) {
        kDebug(285) << "WARNING: changing semantics of keyDoesNotExist with KSS: will prompt for the password";
        Wallet *w = openWallet( wallet, 0, Synchronous );
        if ( w ) {
            return !w->hasEntry(key);
        }
        return false;
    }
    else {
        QDBusReply<bool> r = walletLauncher->getInterface().keyDoesNotExist(wallet, folder, key);
        if (!r.isValid())
        {
            kDebug(285) << "Invalid DBus reply: " << r.error();
            return false;
        }
        else
            return r;
    }
}

void Wallet::virtual_hook(int, void*) {
    //BASE::virtual_hook( id, data );
}


KWalletDLauncher::KWalletDLauncher()
    : m_wallet(0),
    m_cgroup(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet"))
{
    m_useKSecretsService = m_cgroup.readEntry("UseKSecretsService", false);
    if (m_useKSecretsService) {
        // NOOP
    }
    else {
        m_wallet = new org::kde::KWallet(QString::fromLatin1(s_kwalletdServiceName), "/modules/kwalletd", QDBusConnection::sessionBus());
    }
}

KWalletDLauncher::~KWalletDLauncher()
{
    delete m_wallet;
}

org::kde::KWallet &KWalletDLauncher::getInterface()
{
    Q_ASSERT(!m_useKSecretsService);
    Q_ASSERT(m_wallet != 0);
    
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

    return *m_wallet;
}

#include "kwallet.moc"

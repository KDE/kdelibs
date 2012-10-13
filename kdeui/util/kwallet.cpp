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
#include "kwalletpluginloader.h"
#include "kwalletplugin.h"
#include "config-kwallet.h"

#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <QtDBus/QtDBus>
#include <ktoolinvocation.h>

#include <assert.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <ksharedconfig.h>
#include <kwindowsystem.h>


namespace KWallet
{

class KWalletDLauncher
{
public:
    KWalletDLauncher();
    ~KWalletDLauncher();

    // this static variable is used below to switch between old KWallet
    // infrastructure and the new one which is built on top of the new
    // KSecretsService infrastructure. It's value can be changed via the 
    // the Wallet configuration module in System Settings
    bool m_useKSecretsService;
    KConfigGroup m_cgroup;
    WalletPluginLoader m_pluginLoader;
    WalletPlugin *m_plugin;
};


K_GLOBAL_STATIC(KWalletDLauncher, walletLauncher)


#define PLUGIN() walletLauncher->m_plugin

bool Wallet::isUsingKSecretsService()
{
    KConfigGroup cfg(KSharedConfig::openConfig("kwalletrc")->group("Wallet"));
    return cfg.readEntry("UseKSecretsService", false);
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
    WalletPrivate(Wallet *wallet, WalletPlugin *plugin)
     : m_q(wallet), m_plugin(plugin)
    {}

    void walletServiceUnregistered();
    void name();

#ifdef HAVE_KSECRETSSERVICE
    template <typename T> 
    int writeEntry( const QString& key, const T &value, Wallet::EntryType entryType ) {
        int rc = -1;
        KSecretsService::Secret secret;
        secret.setValue( QVariant::fromValue<T>(value) );

        KSecretsService::StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = folder;
        attrs[KSS_ATTR_WALLETTYPE] = QString("%1").arg((int)entryType);
        KSecretsService::CreateCollectionItemJob *createItemJob = secretsCollection->createItem( key, attrs, secret );

        if ( !createItemJob->exec() ) {
            kDebug(285) << "Cannot execute CreateCollectionItemJob : " << createItemJob->errorString();
        }
        rc = createItemJob->error();
        return rc;
    }

    QExplicitlySharedDataPointer<KSecretsService::SecretItem> findItem( const QString& key ) const;
    template <typename T> int readEntry( const QString& key, T& value ) const;
    bool readSecret( const QString& key, KSecretsService::Secret& value ) const;
    
    template <typename V>
    int forEachItemThatMatches( const QString &key, V verb ) {
        int rc = -1;
        KSecretsService::StringStringMap attrs;
        attrs[KSS_ATTR_ENTRYFOLDER] = folder;
        KSecretsService::SearchCollectionItemsJob *searchItemsJob = secretsCollection->searchItems(attrs);
        if ( searchItemsJob->exec() ) {
            QRegExp re(key, Qt::CaseSensitive, QRegExp::Wildcard);
            foreach( KSecretsService::SearchCollectionItemsJob::Item item , searchItemsJob->items() ) {
                KSecretsService::ReadItemPropertyJob *readLabelJob = item->label();
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
            kDebug(285) << "Cannot execute KSecretsService::SearchCollectionItemsJob " << searchItemsJob->errorString();
        }
        return rc;
    }

    void createDefaultFolders();

    struct InsertIntoEntryList;
    struct InsertIntoMapList;
    struct InsertIntoPasswordList;

    KSecretsService::Collection *secretsCollection;
#endif // HAVE_KSECRETSSERVICE

    WalletPlugin *m_plugin;
    Wallet *m_q;
};

#ifdef HAVE_KSECRETSSERVICE
void Wallet::WalletPrivate::createDefaultFolders()
{
// NOTE: KWalletManager expects newly created wallets to have two default folders
//     b->createFolder(KWallet::Wallet::PasswordFolder());
//     b->createFolder(KWallet::Wallet::FormDataFolder());
    QString strDummy("");
    folder = PasswordFolder();
    writeEntry( PasswordFolder(), strDummy, KWallet::Wallet::Unknown );
    
    folder = FormDataFolder();
    writeEntry( FormDataFolder(), strDummy, KWallet::Wallet::Unknown );
}
#endif // HAVE_KSECRETSSERVICE

Wallet::Wallet(WalletPlugin *plugin)
    : QObject(0L), d(new WalletPrivate(this, plugin))
{
}


Wallet::~Wallet() {
#ifdef HAVE_KSECRETSSERVICE
    if (walletLauncher->m_useKSecretsService) {
        d->folder.clear();
        d->name.clear();
        delete d->secretsCollection;
    }
    else {
#endif
#ifdef HAVE_KSECRETSSERVICE
    }
#endif
    delete d;
}


QStringList Wallet::walletList() {
    QStringList result;
    return PLUGIN()->walletList();
}


void Wallet::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::changePassword().";

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();
    PLUGIN()->changePassword( name, w );
    return;
}


bool Wallet::isEnabled() {
    return PLUGIN()->isEnabled();
}


bool Wallet::isOpen(const QString& name) {
    return PLUGIN()->isOpen( name );
}

int Wallet::closeWallet(const QString& name, bool force) {
    return PLUGIN()->closeWallet( name, force );
}


int Wallet::deleteWallet(const QString& name) {
    return PLUGIN()->deleteWallet( name );
}

Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::openWallet().";

    return PLUGIN()->openWallet( name, w, (WalletPlugin::OpenType)ot );
}

bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
    return PLUGIN()->disconnectApplication( wallet, app );
}


QStringList Wallet::users(const QString& name) {
    return PLUGIN()->users( name );
}


int Wallet::sync() {
    PLUGIN()->sync();
    return 0;
}


int Wallet::lockWallet() {
    return PLUGIN()->lockWallet();
}


const QString& Wallet::walletName() const {
    return PLUGIN()->walletName();
}


bool Wallet::isOpen() const {
    return PLUGIN()->isOpen();
}


void Wallet::requestChangePassword(WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";

    PLUGIN()->requestChangePassword(w);
    return;
}



QStringList Wallet::folderList() {
    return PLUGIN()->folderList();
}


QStringList Wallet::entryList() {
    return PLUGIN()->entryList();
}


bool Wallet::hasFolder(const QString& f) {
    return PLUGIN()->hasFolder( f );
}


bool Wallet::createFolder(const QString& f) {
    return PLUGIN()->createFolder( f );
}


bool Wallet::setFolder(const QString& f) {
    return PLUGIN()->setFolder( f );
}


bool Wallet::removeFolder(const QString& f) {
    return PLUGIN()->removeFolder( f );
}


const QString& Wallet::currentFolder() const {
    return PLUGIN()->currentFolder();
}

int Wallet::readEntry(const QString& key, QByteArray& value) {
    return PLUGIN()->readEntry( key, value );
}

int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {

    return PLUGIN()->readEntryList( key, value );
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
    return PLUGIN()->renameEntry( oldName, newName );
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
    return PLUGIN()->readMap( key, value );
}

int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    return PLUGIN()->readMapList( key, value );
}


int Wallet::readPassword(const QString& key, QString& value) {
    return PLUGIN()->readPassword( key, value );
}

int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    return PLUGIN()->readPasswordList( key, value );
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    return PLUGIN()->writeEntry( key, value, (WalletPlugin::OpenType)entryType);
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
    return PLUGIN()->writeEntry( key, value);
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
    return PLUGIN()->writeMap( key, value );
}


int Wallet::writePassword(const QString& key, const QString& value) {
    return PLUGIN()->writePassword( key, value );
}


bool Wallet::hasEntry(const QString& key) {
    return PLUGIN()->hasEntry( key );
}


int Wallet::removeEntry(const QString& key) {
    return PLUGIN()->removeEntry( key );
}


Wallet::EntryType Wallet::entryType(const QString& key) {
    return (Wallet::OpenType) PLUGIN()->entryType( key );
}


void Wallet::WalletPrivate::walletServiceUnregistered()
{
    if (m_loop) {
        m_loop->quit();
    }

    if (m_handle >= 0) {
        q->slotWalletClosed(handle);
    }
}


void Wallet::walletAsyncOpened(int tId, int handle) {
#ifdef HAVE_KSECRETSSERVICE
    if (walletLauncher->m_useKSecretsService) {
        // TODO: implement this
        Q_ASSERT(0);
    }
    else {
#endif
        // ignore responses to calls other than ours
        if (d->transactionId != tId || d->handle != -1) {
            return;
        }

        // disconnect the async signal
        disconnect(this, SLOT(walletAsyncOpened(int,int)));

        d->handle = handle;
        emit walletOpened(handle > 0);
#ifdef HAVE_KSECRETSSERVICE
    }
#endif
}

void Wallet::emitWalletAsyncOpenError() {
    emit walletOpened(false);
}

void Wallet::emitWalletOpened() {
  emit walletOpened(true);
}

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    return PLUGIN()->folderDoesNotExist( wallet, folder );
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    return PLUGIN()->keyDoesNotExist( wallet, folder, key );
}

void Wallet::virtual_hook(int, void*) {
    //BASE::virtual_hook( id, data );
}


KWalletDLauncher::KWalletDLauncher()
    : m_plugin(0)
    m_cgroup(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet"))
{
    // TODO: plugin loading should occur here
    m_useKSecretsService = m_cgroup.readEntry("UseKSecretsService", false);
    if (m_useKSecretsService) {
         m_plugin = m_pluginLoader.loadKSecrets();
    }
    else {
        m_plugin = m_pluginLoader.loadKWallet();
    }
}

KWalletDLauncher::~KWalletDLauncher()
{
    delete m_wallet;
}

} // namespace KWallet

#include "kwallet.moc"

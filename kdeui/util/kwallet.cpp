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
    WalletPrivate() : m_plugin(0) {}
    WalletPrivate(Wallet *wallet, WalletPlugin *plugin) : 
        m_wallet(wallet), 
        m_plugin(plugin)
    {
        Q_ASSERT(m_plugin == plugin);
//        connect( m_plugin, SIGNAL(
    }

    ~WalletPrivate() {
        delete m_plugin;
    }

    void name();

    static WalletPlugin *getStaticPlugin();
    WalletPlugin* getPlugin();

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

    /**
     * This method instantiates a wallet plugin instance followin the
     * global wallet configuration
     */
    static WalletPlugin* createPluginInstance();

private:

    /**
     * This is the generic plugin, used for implementing the static
     * Wallet methods, that do not need any binding to a wallet instance
     */
    static WalletPlugin *m_staticPlugin;
    Wallet *m_wallet;
    /**
     * This is the actual plugin implementing the non-static methods
     * of the main Wallet class
     */
    WalletPlugin *m_plugin;
protected:
    void m_loop();
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

WalletPlugin* Wallet::WalletPrivate::createPluginInstance() {
    WalletPlugin *plugin =0;
    KConfigGroup cgroup = KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)->group("Wallet");
    bool useKSecretsService = cgroup.readEntry("UseKSecretsService", false);
    if (useKSecretsService) {
        plugin = WalletPluginLoader::instance()->loadKSecrets();
    }
    else {
        plugin = WalletPluginLoader::instance()->loadKWallet();
    }
    return plugin;
}

WalletPlugin* Wallet::WalletPrivate::m_staticPlugin =0;

WalletPlugin *Wallet::WalletPrivate::getStaticPlugin() {
    if (0 == m_staticPlugin) {
        m_staticPlugin = createPluginInstance();
    }
    return m_staticPlugin;
}

WalletPlugin* Wallet::WalletPrivate::getPlugin() {
    if ( 0 == m_plugin ) {
        m_plugin = createPluginInstance();
    }
    return m_plugin;
}

Wallet::Wallet(WalletPlugin *plugin)
    : QObject(0L), d(new WalletPrivate(this, plugin))
{
    // we'll now connect the signals of the plugin to the signals of the wallet
    // this will allow triggerring wallet signals when the plugin emit these signals
    // without needing intermediary slots into kwallet
    connect( plugin, SIGNAL(walletOpened(bool)), this, SLOT(emitWalletOpened()) );
    connect( plugin, SIGNAL(walletClosed()), this, SLOT(emitWalletClosed()) );
    connect( plugin, SIGNAL(folderUpdated(const QString&)), this, SLOT(emitFolderUpdated(const QString&)) );
    connect( plugin, SIGNAL(folderListUpdated()), this, SLOT(emitFolderListUpdated()) );
    connect( plugin, SIGNAL(folderRemoved(const QString&)), this, SLOT(emitFolderRemoved(const QString&)) );
}


Wallet::~Wallet() {
    delete d;
}

QStringList Wallet::walletList() {
    return WalletPrivate::getStaticPlugin()->walletList();
}

void Wallet::changePassword(const QString& name, WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::changePassword().";

    // Make sure the password prompt window will be visible and activated
    KWindowSystem::allowExternalProcessWindowActivation();
    WalletPrivate::getStaticPlugin()->changePassword( name, w );
}


bool Wallet::isEnabled() {
    return WalletPrivate::getStaticPlugin()->isEnabled();
}


bool Wallet::isOpen(const QString& name) {
    return WalletPrivate::getStaticPlugin()->isOpen( name );
}

int Wallet::closeWallet(const QString& name, bool force) {
    return WalletPrivate::getStaticPlugin()->closeWallet( name, force );
}


int Wallet::deleteWallet(const QString& name) {
    return WalletPrivate::getStaticPlugin()->deleteWallet( name );
}

Wallet *Wallet::openWallet(const QString& name, WId w, OpenType ot) {
    Wallet *result =0;
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::openWallet().";

    WalletPlugin *plugin = WalletPrivate::createPluginInstance();
    if (plugin->openWallet( name, w, (WalletPlugin::OpenType)ot )) {
        result = new Wallet( plugin );
    }
    else {
        delete plugin;
    }

    return result;
}

bool Wallet::disconnectApplication(const QString& wallet, const QString& app) {
    return WalletPrivate::getStaticPlugin()->disconnectApplication( wallet, app );
}


QStringList Wallet::users(const QString& name) {
    return WalletPrivate::getStaticPlugin()->users( name );
}


int Wallet::sync() {
    d->getPlugin()->sync();
    return 0;
}


int Wallet::lockWallet() {
    return d->getPlugin()->lockWallet();
}


const QString& Wallet::walletName() const {
    return d->getPlugin()->walletName();
}


bool Wallet::isOpen() const {
    return d->getPlugin()->isOpen();
}


void Wallet::requestChangePassword(WId w) {
    if( w == 0 )
        kDebug(285) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";

    d->getPlugin()->requestChangePassword(w);
    return;
}

void Wallet::slotWalletClosed(int handle) {
    // NOTE implementation moved to WalletDefaultPlugin::slotWalletClosed
}


QStringList Wallet::folderList() {
    return d->getPlugin()->folderList();
}


QStringList Wallet::entryList() {
    return d->getPlugin()->entryList();
}


bool Wallet::hasFolder(const QString& f) {
    return d->getPlugin()->hasFolder( f );
}


bool Wallet::createFolder(const QString& f) {
    return d->getPlugin()->createFolder( f );
}


bool Wallet::setFolder(const QString& f) {
    return d->getPlugin()->setFolder( f );
}


bool Wallet::removeFolder(const QString& f) {
    return d->getPlugin()->removeFolder( f );
}


const QString& Wallet::currentFolder() const {
    return d->getPlugin()->currentFolder();
}

int Wallet::readEntry(const QString& key, QByteArray& value) {
    return d->getPlugin()->readEntry( key, value );
}

int Wallet::readEntryList(const QString& key, QMap<QString, QByteArray>& value) {

    return d->getPlugin()->readEntryList( key, value );
}


int Wallet::renameEntry(const QString& oldName, const QString& newName) {
    return d->getPlugin()->renameEntry( oldName, newName );
}


int Wallet::readMap(const QString& key, QMap<QString,QString>& value) {
    return d->getPlugin()->readMap( key, value );
}

int Wallet::readMapList(const QString& key, QMap<QString, QMap<QString, QString> >& value) {
    return d->getPlugin()->readMapList( key, value );
}


int Wallet::readPassword(const QString& key, QString& value) {
    return d->getPlugin()->readPassword( key, value );
}

int Wallet::readPasswordList(const QString& key, QMap<QString, QString>& value) {
    return d->getPlugin()->readPasswordList( key, value );
}


int Wallet::writeEntry(const QString& key, const QByteArray& value, EntryType entryType) {
    return d->getPlugin()->writeEntry( key, value, (WalletPlugin::EntryType)entryType);
}


int Wallet::writeEntry(const QString& key, const QByteArray& value) {
    return d->getPlugin()->writeEntry( key, value);
}


int Wallet::writeMap(const QString& key, const QMap<QString,QString>& value) {
    return d->getPlugin()->writeMap( key, value );
}


int Wallet::writePassword(const QString& key, const QString& value) {
    return d->getPlugin()->writePassword( key, value );
}


bool Wallet::hasEntry(const QString& key) {
    return d->getPlugin()->hasEntry( key );
}


int Wallet::removeEntry(const QString& key) {
    return d->getPlugin()->removeEntry( key );
}


Wallet::EntryType Wallet::entryType(const QString& key) {
    return (Wallet::EntryType) d->getPlugin()->entryType( key );
}

void Wallet::slotFolderUpdated(const QString& wallet, const QString& folder) {
    // NOTE implementation moved to WalletDefaultPlugin::slotFolderUpdated
}


void Wallet::slotFolderListUpdated(const QString& wallet) {
    // NOTE implementation moved to WalletDefaultPlugin::slotFolderListUpdated
}


void Wallet::slotApplicationDisconnected(const QString& wallet, const QString& application) {
    // NOTE implementation moved to WalletDefaultPlugin::slotApplicationDisconnected
}

void Wallet::walletAsyncOpened(int tId, int handle) {
    // NOTE implementation moved to WalletDefaultPlugin::walletAsyncOpened
}

void Wallet::emitWalletAsyncOpenError() {
    emit walletOpened(false);
}

void Wallet::emitWalletOpened() {
    emit walletOpened(true);
}

void Wallet::emitFolderListUpdated()
{
    emit folderListUpdated();
}

void Wallet::emitFolderRemoved(const QString& folderName)
{
    emit folderRemoved( folderName );
}

void Wallet::emitFolderUpdated(const QString& folderName)
{
    emit folderUpdated( folderName );
}

void Wallet::emitWalletClosed()
{
    emit walletClosed();
}

bool Wallet::folderDoesNotExist(const QString& wallet, const QString& folder)
{
    return WalletPrivate::getStaticPlugin()->folderDoesNotExist( wallet, folder );
}


bool Wallet::keyDoesNotExist(const QString& wallet, const QString& folder, const QString& key)
{
    return WalletPrivate::getStaticPlugin()->keyDoesNotExist( wallet, folder, key );
}

void Wallet::virtual_hook(int, void*) {
    //BASE::virtual_hook( id, data );
}

void Wallet::slotCollectionStatusChanged( int ) {
    // not used
}

void Wallet::slotCollectionDeleted() {
    // not used
}

} // namespace KWallet

#include "kwallet.moc"

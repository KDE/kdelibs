/*
    knewstuff3/engine.h.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_ENGINE_H
#define KNEWSTUFF3_ENGINE_H

#include <knewstuff3/core/provider.h>
#include <knewstuff3/staticxml/entry.h>

#include <knewstuff3/knewstuff_export.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

class KJob;
class KArchiveDirectory;

namespace KNS3
{

class Installation;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 *
 * @internal
 */
class KNEWSTUFF_EXPORT Engine : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    Engine(QObject* parent);

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~Engine();

    /**
     * Policy on how to cache the data received from the network. While
     * CacheNever completely switches off all caching, the other two settings
     * CacheReplaceable and CacheResident will optimize the network traffic
     * needed for all workflows.
     * CacheOnly will never download from the network at all and can be used
     * to inspect the local cache.
     *
     * Provider files, feeds, entries and preview images are subject to this
     * policy.
     *
     * The default cache policy is CacheNever.
     */
    enum CachePolicy {
        /**< Do not use any cache. (default) */
        CacheNever,
        /**< Use the cache first, but then update from the network. */
        CacheReplaceable,
        /**< Like CacheReplaceable, but only update if necessary. */
        CacheResident,
        /**< Operate on cache files but never update them. */
        CacheOnly
    };

    /**
     * Initializes the engine. This step is application-specific and relies
     * on an external configuration file, which determines all the details
     * about the initialization.
     *
     * @param configfile KNewStuff2 configuration file (*.knsrc)
     * @return \b true if any valid configuration was found, \b false otherwise
     */
    bool init(const QString &configfile);

    /**
     * Starts the engine. This method reports all cached and registered
     * providers to the application. Depending on the cache policy,
     * the engine will then try to synchronize the cache by updating
     * all information about the providers.
     *
     * If engine automation is activated, this method will proceed to
     * synchronize all feeds, entries and preview images.
     * For each provider, all feeds are considered. The synchronization is
     * complete if \ref signalEntriesFinished is emitted, but applications
     * should continue watching \ref signalEntryChanged.
     *
     * @see signalProviderLoaded
     * @see signalProviderChanged
     * @see signalProvidersFailed
     * @see signalProvidersFinished
     * @see signalEntryLoaded
     * @see signalEntryChanged
     * @see signalEntriesFailed
     * @see signalEntriesFinished
     * @see signalEntriesFeedFinished
     * @see signalPreviewLoaded
     * @see signalPreviewFailed
     */
    //void start();

    /**
     * Loads all entries of all the feeds from a provider. This means that
     * meta information about those entries is retrieved from the cache and/or
     * from the network, depending on the cache policy.
     *
     * This method should not be called if automation is activated.
     *
     * @param provider Provider from where to load the entries
     *
     * @see signalEntryLoaded
     * @see signalEntryChanged
     * @see signalEntriesFailed
     * @see signalEntriesFinished
     * @see signalEntriesFeedFinished
     */
    //void loadEntries(Provider *provider);
    //void loadProvider(); // FIXME: for consistency?

    /**
     * Downloads a preview file. The preview file matching most closely
     * the current user language preferences will be downloaded.
     *
     * This method should not be called if automation is activated.
     *
     * @param entry Entry to download preview image for
     *
     * @see signalPreviewLoaded
     * @see signalPreviewFailed
     */
    //void downloadPreview(Entry *entry);

    /**
     * Downloads a payload file. The payload file matching most closely
     * the current user language preferences will be downloaded.
     * The file will not be installed set, for this \ref install must
     * be called.
     *
     * @param entry Entry to download payload file for
     *
     * @see signalPayloadLoaded
     * @see signalPayloadFailed
     */
    void downloadPayload(Entry *entry);

    /**
     * Uploads a complete entry, including its payload and preview files
     * (if present) and all associated meta information.
     * Note that this method is asynchronous and thus the return value will
     * not report the final success of all upload steps. It will merely check
     * that the provider supports upload and so forth.
     *
     * @param provider Provider to use for upload
     * @param entry Entry to upload with associated files
     *
     * @return Whether or not upload was started successfully
     *
     * @see signalEntryUploaded
     * @see signalEntryFailed
     */
    bool uploadEntry(Provider *provider, Entry *entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     * Note that this method is asynchronous and thus the return value will
     * only report the successful start of the installation.
     *
     * @param payloadfile Path to file to install
     *
     * @return Whether or not installation was started successfully
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     *
     * @note FIXME: use Entry as parameter
     */
    bool install(const QString& payloadfile);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     *
     * @return Whether or not deinstallation was successful
     *
     * @note FIXME: I don't believe this works yet :)
     */
    bool uninstall(KNS3::Entry *entry);

    /**
     * @return the component name the engine is using, or an empty string if not
     * initialized yet
     */
    QString componentName() const;

Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString& message);

    /**
     * Indicates that the list of providers has been successfully loaded.
     * This signal might occur twice, for the local cache and for updated provider
     * information from the ProvidersUrl.
     */
    void signalProviderLoaded(KNS3::Provider *provider);
    void signalProviderChanged(KNS3::Provider *provider);
    void signalProvidersFailed();

    //void signalEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    //void signalEntryRemoved(KNS::Entry *entry, const KNS::Feed *feed);
    void signalEntryChanged(KNS3::Entry *entry);
    void signalEntriesFailed();

    void signalPreviewLoaded(KUrl preview); // FIXME: return Entry
    void signalPreviewFailed();

    void signalPayloadLoaded(KUrl payload); // FIXME: return Entry
    void signalPayloadFailed(KNS3::Entry *entry);

    void signalEntryUploaded(); // FIXME: rename to signalEntryUploadFinished?
    void signalEntryFailed(); // FIXME: rename to signalEntryUploadFailed?

    void signalProvidersFinished();
    void signalEntriesFinished();
    //void signalEntriesFeedFinished(const KNS::Feed *feed);

    void signalInstallationFinished();
    void signalInstallationFailed();

    void signalProgress(const QString & message, int percentage);

    void signalDownloadDialogDone(KNS3::Entry::List);
protected:
    //void mergeEntries(Entry::List entries, Feed *feed, const Provider *provider);
private Q_SLOTS:
    void slotProvidersLoaded(const QDomDocument& doc);
    void slotProvidersFailed();

    void slotEntriesLoaded(KNS3::Entry::List list);
    void slotEntriesFailed();

    void slotPayloadResult(KJob *job);
    void slotPreviewResult(KJob *job);

    void slotUploadPayloadResult(KJob *job);
    void slotUploadPreviewResult(KJob *job);
    void slotUploadMetaResult(KJob *job);

    void slotProgress(KJob *job, unsigned long percent);

    void slotInstallationVerification(int result);

private:
    /*
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();
    
    void loadRegistry();
    void loadProvidersCache();
    KNS3::Entry *loadEntryCache(const QString& filepath);
#if 0
    void loadEntriesCache();
#endif
    void loadFeedCache(Provider *provider);
    void cacheProvider(Provider *provider);
    void cacheEntry(Entry *entry);

    /** generate a cache file for the given feed
      feed cache file is a list of entry-id's that are part of this feed
      @param provider the provider the feed comes from
      @param feedname the name of the feed, used for keying the feed cache filename
      @param feed feed to be cached
      @param entries entries to cache in the feed file
    */
    //void cacheFeed(const Provider *provider, const QString & feedname, const Feed *feed, Entry::List entries);
    void registerEntry(Entry *entry);
    void unregisterEntry(Entry *entry);
    void mergeProviders(Provider::List providers);
    void shutdown();

    bool entryCached(Entry *entry);
    bool entryChanged(Entry *oldentry, Entry *entry);
    bool providerCached(Provider *provider);
    bool providerChanged(Provider *oldprovider, Provider *provider);

    static QStringList archiveEntries(const QString& path, const KArchiveDirectory * dir);

    QString id(Entry *e);
    QString pid(const Provider *p);

    QList<Provider*> m_provider_cache;

    // holds all the entries
    QList<Entry*> m_entry_cache;

    // holds the registered entries mapped by their id
    QMap<QString, Entry*> m_entry_registry;

    QMap<QString, Provider*> m_provider_index;
    QMap<QString, Entry*> m_entry_index;

    Entry *m_uploadedentry;
    Provider *m_uploadprovider;

    QString m_providersurl;

    // the name of the app that uses hot new stuff
    QString m_applicationName;

    QMap<Entry*, QString> m_previewfiles;
    QMap<Entry*, QString> m_payloadfiles;

    QMap<KJob*, Entry*> m_entry_jobs;

    Installation *m_installation;

    int m_activefeeds;

    bool m_initialized;
    CachePolicy m_cachepolicy;
};

}

#endif

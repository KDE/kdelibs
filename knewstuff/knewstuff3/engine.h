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
#include <knewstuff3/core/entry.h>

#include <knewstuff3/knewstuff_export.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

class KJob;

namespace KNS3
{

class Installation;
class EnginePrivate;

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
    Engine(QObject* parent = 0);

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~Engine();


    // FIXME move into Cache
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

    enum CollaborationFeature {
        None = 0x0,
        Ratings = 0x1,
        Comments = 0x2
    };
    Q_DECLARE_FLAGS(CollaborationFeatures, CollaborationFeature)



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
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     *
     * @param entry Entry to be installed
     *
     * @return Whether or not installation was started successfully
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(const Entry& entry);

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
    void uninstall(const Entry& entry);
    
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
    bool uploadEntry(Provider *provider, const Entry& entry);


    /**
     * @return the component name the engine is using, or an empty string if not
     * initialized yet
     */
    QString componentName() const;


    CollaborationFeatures collaborationFeatures(const Entry& entry);

    void setSortMode(Provider::SortMode mode);
    void setSearchTerm(const QString& searchString);
    void reloadEntries();
    
Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString& message);

    // FIXME I think all of the methods below can be removed - no one should care about providers (?)
    /**
     * Indicates that the list of providers has been successfully loaded.
     * This signal might occur twice, for the local cache and for updated provider
     * information from the ProvidersUrl.
     */
    void signalProviderLoaded(KNS3::Provider *provider);
    void signalProviderChanged(KNS3::Provider *provider);

    void signalEntriesLoaded(KNS3::Entry::List entries);

    //void signalEntryRemoved(KNS::Entry *entry, const KNS::Feed *feed);
    void signalEntryChanged(const KNS3::Entry& entry);

    void signalPreviewLoaded(KUrl preview); // FIXME: return Entry
    void signalPreviewFailed();

    void signalEntryUploaded(); // FIXME: rename to signalEntryUploadFinished?
    void signalEntryFailed(); // FIXME: rename to signalEntryUploadFailed?

    void signalProvidersFinished();
    void signalEntriesFinished();
    
    void signalProgress(const QString & message, int percentage);

    void signalDownloadDialogDone(KNS3::Entry::List);

    void signalError(const QString& errorMessage);
    
protected:
    //void mergeEntries(Entry::List entries, Feed *feed, const Provider *provider);
private Q_SLOTS:
    // the .knsrc file was loaded
    void slotProviderFileLoaded(const QDomDocument& doc);
    // loading the .knsrc file failed
    void slotProvidersFailed();
    
    // called when a provider is ready to work
    void providerInitialized(KNS3::Provider*);

    void slotEntriesLoaded(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, Entry::List);

    void slotPreviewResult(KJob *job);

    void slotUploadPayloadResult(KJob *job);
    void slotUploadPreviewResult(KJob *job);
    void slotUploadMetaResult(KJob *job);

    void slotProgress(KJob *job, unsigned long percent);
    
    void slotEntryChanged(const Entry& entry);
    void slotInstallationFailed(const Entry& entry);
    void downloadLinkLoaded(const Entry& entry);
    
private:
    /**
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();

    /// REMOVE THESE
    void loadRegistry();
    void loadProvidersCache();
    KNS3::Entry loadEntryCache(const QString& filepath);

    void loadCache(CachePolicy policy);
    
    // if at all: move into staticprovider:
    //void loadFeedCache(Provider *provider);
    //void cacheProvider(Provider *provider);

    // all entries should be cached, depending on policy on exit:
    void cacheEntry(const Entry& entry);

    /** generate a cache file for the given feed
      feed cache file is a list of entry-id's that are part of this feed
      @param provider the provider the feed comes from
      @param feedname the name of the feed, used for keying the feed cache filename
      @param feed feed to be cached
      @param entries entries to cache in the feed file
    */
    //void cacheFeed(const Provider *provider, const QString & feedname, const Feed *feed, Entry::List entries);
    void registerEntry(const Entry& entry);
    void unregisterEntry(const Entry& entry);
    void shutdown();

    bool entryCached(const Entry& entry);
    bool entryChanged(const Entry& oldentry, const Entry& entry);
    bool providerCached(Provider *provider);

    /**
     * Private copy constructor
     */
    Engine(const Engine& other);
    class Private;
    Private* const d;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Engine::CollaborationFeatures)
} 

#endif

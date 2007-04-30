/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KNEWSTUFF2_COREENGINE_H
#define KNEWSTUFF2_COREENGINE_H

#include <knewstuff2/core/provider.h>
#include <knewstuff2/core/entry.h>

#include <knewstuff2/knewstuff_export.h>

#include <QtCore/QObject>
#include <QtCore/QCharRef>
#include <QtCore/QMap>

class KJob;

namespace KNS {

class ProviderLoader;
class EntryLoader;
class Installation;

/**
 * KNewStuff core engine.
 * A core engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 */
class KNEWSTUFF_EXPORT CoreEngine : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructor.
     */
    CoreEngine();

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~CoreEngine();

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
     * providers and entries to the application. If localonly is set to
     * \b false, the engine will then try to synchronize the cache by updating
     * all information about the providers and entries.
     * For each provider, all feeds are considered. The synchronization is
     * complete if \ref signalEntriesFinished is emitted, but applications
     * should continue watching \ref signalEntryChanged.
     *
     * @param localonly Whether or not to restrict the engine to cache loading
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
     *
     * @note FIXME: Currently, all the entry stuff only happens after
     * loadEntries. It seems smart to let start take care of that and
     * make loadEntries private.
     */
    void start(bool localonly);

    // see start(bool) above!
    void loadEntries(Provider *provider);

    /**
     * Downloads a preview file. The preview file matching most closely
     * the current user language preferences will be downloaded.
     *
     * @param entry Entry to download preview image for
     *
     * @see signalPreviewLoaded
     * @see signalPreviewFailed
     */
    void downloadPreview(Entry *entry);

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
    bool install(QString payloadfile);

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
    bool uninstall(KNS::Entry *entry);

  Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     * This signal might occur twice, for the local cache and for updated provider
     * information from the ProvidersUrl.
     */
    void signalProviderLoaded(KNS::Provider *provider);
    void signalProviderChanged(KNS::Provider *provider);
    void signalProvidersFailed();
    void signalEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void signalEntryChanged(KNS::Entry *entry);
    void signalEntriesFailed();
    void signalPreviewLoaded(KUrl preview); // FIXME: return Entry
    void signalPreviewFailed();
    void signalPayloadLoaded(KUrl payload); // FIXME: return Entry
    void signalPayloadFailed();
    void signalEntryUploaded(); // FIXME: rename to signalEntryUploadFinished?
    void signalEntryFailed(); // FIXME: rename to signalEntryUploadFailed?

    void signalProvidersFinished();
    void signalEntriesFinished();
    void signalEntriesFeedFinished(const KNS::Feed *feed);

    void signalInstallationFinished();
    void signalInstallationFailed();

  private Q_SLOTS:
    void slotProvidersLoaded(KNS::Provider::List list);
    void slotProvidersFailed();
    void slotEntriesLoaded(KNS::Entry::List list);
    void slotEntriesFailed();
    void slotPayloadResult(KJob *job);
    void slotPreviewResult(KJob *job);
    void slotUploadPayloadResult(KJob *job);
    void slotUploadPreviewResult(KJob *job);
    void slotUploadMetaResult(KJob *job);

    void slotInstallationVerification(int result);

  private:
    void loadRegistry(const QString &registrydir);
    void loadProvidersCache();
    KNS::Entry *loadEntryCache(const QString& filepath);
    void loadEntriesCache();
    void loadFeedCache(Provider *provider);
    void cacheProvider(Provider *provider);
    void cacheEntry(Entry *entry);
    void cacheFeed(const Provider *provider, QString feedname, const Feed *feed, Entry::List entries);
    void registerEntry(Entry *entry);
    void mergeProviders(Provider::List providers);
    void mergeEntries(Entry::List entries, const Feed *feed, const Provider *provider);
    void shutdown();

    bool entryCached(Entry *entry);
    bool entryChanged(Entry *oldentry, Entry *entry);
    bool providerCached(Provider *provider);
    bool providerChanged(Provider *oldprovider, Provider *provider);

    QString id(Entry *e);
    QString pid(const Provider *p);

    QList<Provider*> m_provider_cache;
    QList<Entry*> m_entry_cache;

    QMap<QString, Provider*> m_provider_index;
    QMap<QString, Entry*> m_entry_index;

    Entry *m_uploadedentry;
    Provider *m_uploadprovider;

    QString m_providersurl;
    QString m_localregistrydir;
    QString m_componentname;

    QMap<Entry*, QString> m_previewfiles;
    QMap<Entry*, QString> m_payloadfiles;

    QMap<KJob*, Entry*> m_entry_jobs;

    Installation *m_installation;

    int m_activefeeds;

    bool m_initialized;
};

}

#endif

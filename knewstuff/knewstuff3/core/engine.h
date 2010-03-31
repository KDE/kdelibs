/*
    knewstuff3/engine.h.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

#include "provider.h"
#include "entryinternal.h"

class QTimer;
class KJob;

namespace Attica {
    class ProviderManager;
    class Provider;
}

namespace KNS3
{
class Cache;
class Installation;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 *
 * @internal
 */
class Engine : public QObject
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
    void install(KNS3::EntryInternal entry, int linkId = 1);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    void uninstall(KNS3::EntryInternal entry);

    void loadPreview(const KNS3::EntryInternal& entry, EntryInternal::PreviewType type);
    void loadDetails(const KNS3::EntryInternal& entry);

    void setSortMode(Provider::SortMode mode);

    /**
      Set the categories that will be included in searches
      */
    void setCategoriesFilter(const QStringList& categories);
    void setSearchTerm(const QString& searchString);
    void reloadEntries();
    void requestMoreData();
    void requestData(int page, int pageSize);

    void checkForUpdates();

    /**
     * Try to contact the author of the entry by email or showing their homepage.
     */
    void contactAuthor(const EntryInternal& entry);

    bool userCanVote(const EntryInternal& entry);
    void vote(const EntryInternal& entry, bool positiveVote);
    bool userCanBecomeFan(const EntryInternal& entry);
    void becomeFan(const EntryInternal& entry);

    QStringList categories() const;
    QStringList categoriesFilter() const;

Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString& message);

    void signalProvidersLoaded();
    void signalEntriesLoaded(const KNS3::EntryInternal::List& entries);
    void signalUpdateableEntriesLoaded(const KNS3::EntryInternal::List& entries);
    void signalEntryChanged(const KNS3::EntryInternal& entry);
    void signalEntryDetailsLoaded(const KNS3::EntryInternal& entry);

    // a new search result is there, clear the list of items
    void signalResetView();

    void signalEntryPreviewLoaded(const KNS3::EntryInternal&, KNS3::EntryInternal::PreviewType);
    void signalPreviewFailed();

    void signalEntryUploadFinished();
    void signalEntryUploadFailed();

    void signalDownloadDialogDone(KNS3::EntryInternal::List);
    void jobStarted(KJob*,const QString&);

    void signalError(const QString&);
    void signalBusy(const QString&);
    void signalIdle(const QString&);

private Q_SLOTS:
    // the .knsrc file was loaded
    void slotProviderFileLoaded(const QDomDocument& doc);
    // instead of getting providers from knsrc, use what was configured in ocs systemsettings
    void atticaProviderLoaded(const Attica::Provider& provider);

    // loading the .knsrc file failed
    void slotProvidersFailed();

    // called when a provider is ready to work
    void providerInitialized(KNS3::Provider*);

    void slotEntriesLoaded(const KNS3::Provider::SearchRequest&, KNS3::EntryInternal::List);
    void slotEntryDetailsLoaded(const KNS3::EntryInternal& entry);
    void slotPreviewLoaded(const KNS3::EntryInternal& entry, KNS3::EntryInternal::PreviewType type);

    void slotSearchTimerExpired();

    void slotEntryChanged(const KNS3::EntryInternal& entry);
    void slotInstallationFinished();
    void slotInstallationFailed(const QString& message);
    void downloadLinkLoaded(const KNS3::EntryInternal& entry);
    
    void providerJobStarted(KJob*);

private:
    /**
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();

    /**
      Add a provider and connect it to the right slots
     */
    void addProvider(QSharedPointer<KNS3::Provider> provider);

    void updateStatus();
    
    void doRequest();

    // If the provider is ready to be used
    bool m_initialized;
    // handle installation of entries
    Installation* m_installation;
    // read/write cache of entries
    Cache* m_cache;
    QTimer* m_searchTimer;
    // The url of the file containing information about content providers
    QString m_providerFileUrl;
    // Categories from knsrc file
    QStringList m_categories;

    QHash<QString, QSharedPointer<KNS3::Provider> > m_providers;

    // the name of the app that uses hot new stuff
    QString m_applicationName;

    // the current request from providers
    Provider::SearchRequest m_currentRequest;

    // the page that is currently displayed, so it is not requested repeatedly
    int m_currentPage;

    // when requesting entries from a provider, how many to ask for
    int m_pageSize;

    int m_numDataJobs;
    int m_numPictureJobs;
    int m_numInstallJobs;
    Attica::ProviderManager* m_atticaProviderManager;

    Q_DISABLE_COPY(Engine)
};

}

#endif

/*
    knewstuff3/engine.h.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2009 Frederik Gladhorn <gladhorn@kde.org>
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
#include "entry.h"

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
class Engine : public QObject
{
    Q_OBJECT
public:
    enum EntryAction {
        ViewInfo,
        Comments,
        Changes,
        ContactEmail,
        ContactJabber,
        CollabTranslate,
        CollabRemoval,
        CollabSubscribe,
        Uninstall,
        Install,
        AddComment,
        Rate,
        ShowDetails
    };
    
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
    void install(KNS3::Entry entry);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    void uninstall(KNS3::Entry entry);

    void setSortMode(Provider::SortMode mode);
    void setSearchTerm(const QString& searchString);
    void reloadEntries();
    void slotRequestMoreData();

public Q_SLOTS:
    void slotPerformAction(KNS3::Engine::EntryAction action, KNS3::Entry entry);
    
Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString& message);

    void signalEntriesLoaded(const KNS3::Entry::List& entries);
    void signalEntryChanged(const KNS3::Entry& entry);

    // a new search result is there, clear the list of items
    void signalResetView();
    
    void signalPreviewLoaded(KUrl preview); // FIXME: return Entry
    void signalPreviewFailed();

    void signalEntryUploadFinished();
    void signalEntryUploadFailed();

    void signalProvidersFinished();
    void signalEntriesFinished();
    
    void signalProgress(const QString & message, int percentage);

    void signalDownloadDialogDone(KNS3::Entry::List);

    void signalError(const QString& errorMessage);
    
    void signalJobStarted(KJob*);
    
private Q_SLOTS:
    // the .knsrc file was loaded
    void slotProviderFileLoaded(const QDomDocument& doc);
    // loading the .knsrc file failed
    void slotProvidersFailed();
    
    // called when a provider is ready to work
    void providerInitialized(KNS3::Provider*);

    void slotEntriesLoaded(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, KNS3::Entry::List);
    
    void slotPreviewResult(KJob *job);

    void slotSearchTimerExpired();

    void slotProgress(KJob *job, unsigned long percent);
    
    void slotEntryChanged(const KNS3::Entry& entry);
    void slotInstallationFailed(const KNS3::Entry& entry);
    void downloadLinkLoaded(const KNS3::Entry& entry);
    
private:
    /**
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();

    void loadRegistry();

    bool entryChanged(const Entry& oldentry, const Entry& entry);

    /**
     * Private copy constructor
     */
    Engine(const Engine& other);
    class Private;
    Private* const d;

    class ProviderInformation;
};

} 

#endif

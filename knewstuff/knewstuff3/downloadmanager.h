/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_DownloadManager_H
#define KNEWSTUFF3_UI_DownloadManager_H

#include "knewstuff_export.h"
#include "entry.h"

namespace KNS3
{

/**
 * KNewStuff update checker.
 * This class can be used to search for KNewStuff items
 * without using the widgets and to look for updates of
 * already installed items without showing the dialog.
 * @since 4.5
 */
class KNEWSTUFF_EXPORT DownloadManager :public QObject
{
    Q_OBJECT

public:

    /**
     * Create a DownloadManager
     * It will try to find a appname.knsrc file with the configuration.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     */
    explicit DownloadManager(QObject * parent = 0);

    /**
     * Create a DownloadManager. Manually specifying the name of the .knsrc file.
     *
     * @param configFile the name of the configuration file
     * @param parent
     */
    explicit DownloadManager(const QString& configFile, QObject * parent = 0);

    /**
     * destructor
     */
    ~DownloadManager();

    /*
      Search for a list of entries. searchResult will be emitted with the requested list.    
    */
    //void search(const QStringList& categories = QStringList(), const QString& searchTerm = QString(), int page = 0, int pageSize = 100);
    
    /**
      Check for available updates.
      Use searchResult to get notified as soon as an update has been found.
      */
    void checkForUpdates();

    /**
      Installs or updates an entry
      @param entry
      */
    void installEntry(const KNS3::Entry& entry);

Q_SIGNALS:
    /**
      Returns the search result.
      This can be the list of updates after checkForUpdates or the result of a search.
      @param entries the list of results. entries is empty when nothing was found.
     */
    void searchResult(const KNS3::Entry::List& entries);
    
    /**
      The entry status has changed: emitted when the entry has been installed, updated or removed.
      Use KNS3::Entry::status() to check the current status.
      @param entry the item that has been updated.
     */
    void entryStatusChanged(const KNS3::Entry& entry);

private:
    void init(const QString& configFile);

    Q_PRIVATE_SLOT( d, void _k_slotProvidersLoaded() )
    Q_PRIVATE_SLOT( d, void _k_slotEntryStatusChanged(const KNS3::EntryInternal& entry) )
    Q_PRIVATE_SLOT( d, void _k_slotUpdatesLoaded(const KNS3::EntryInternal::List& entries) )
    class Private;
    Private *const d;
    Q_DISABLE_COPY(DownloadManager)
};

}

#endif

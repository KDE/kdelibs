/*
    knewstuff3/ui/downloaddialog.h.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_DOWNLOADWIDGET_H
#define KNEWSTUFF3_UI_DOWNLOADWIDGET_H

#include <QWidget>

#include "knewstuff_export.h"
#include "entry.h"

namespace KNS3
{
class DownloadWidgetPrivate;

/**
 * KNewStuff download widget.
 *
 * The download widget will present items to the user
 * for installation, updates and removal.
 * Preview images as well as other meta information can be seen.
 *
 * \section knsrc knsrc Files
 * The Dialog is configured by a .knsrc file containing the KHotNewStuff configuration.
 * Your application should install a file called: <em>$KDEDIR/share/config/appname.knsrc</em>
 *
 * The file could look like this for wallpapers:
 * <pre>
   [KNewStuff3]
   ProvidersUrl=http://download.kde.org/ocs/providers.xml
   Categories=KDE Wallpaper 1920x1200,KDE Wallpaper 1600x1200
   StandardResource=wallpaper
   Uncompress=archive
 * </pre>
 *
 * Uncompress can be one of: always, never or archive:
 * <ol>
 * <li>always: assume all downloaded files are archives and need to be extracted</li>
 * <li>never: never try to extract the file</li>
 * <li>archive: if the file is an archive, uncompress it, otherwise just pass it on</li>
 * </ol>
 *
 * You have different options to set the target install directory:
 *   <ol><li>StandardResource: not available in KF5, use XdgTargetDir instead.</li>
 *       <li>TargetDir: since KF5, this is equivalent to XdgTargetDir.
 *       <li>XdgTargetDir: a directory in the $XDG_DATA_HOME directory such as <em>.local/share/wallpapers</em>.
 *           This is what QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + name will return.</li>
 *   </ol>
 *
 * @since 4.5
 */
class KNEWSTUFF_EXPORT DownloadWidget :public QWidget
{
    Q_OBJECT

public:
    /**
     * Create a download widget that lets the user install, update and uninstall
     * contents. It will try to find a appname.knsrc file with the configuration.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     */
    explicit DownloadWidget(QWidget * parent = 0);

    /**
     * Create a download widget that lets the user install, update and uninstall
     * contents. Manually specify the name of a .knsrc file where the
     * KHotNewStuff configuration can be found.
     *
     * @param configFile the name of the configuration file
     * @param parent the parent of the dialog
     */
    explicit DownloadWidget(const QString& configFile, QWidget * parent = 0);

    /**
     * destructor
     */
    ~DownloadWidget();

    /**
     * The list of entries with changed status (installed/uninstalled)
     * @return the list of entries
     */
    KNS3::Entry::List changedEntries();

    /**
     * The list of entries that have been newly installed
     * @return the list of entries
     */
    KNS3::Entry::List installedEntries();

private:
    void init(const QString& configFile);

    DownloadWidgetPrivate *const d;
    Q_DISABLE_COPY(DownloadWidget)

    Q_PRIVATE_SLOT( d, void slotListViewListMode() )
    Q_PRIVATE_SLOT( d, void slotListViewIconMode() )

    Q_PRIVATE_SLOT( d, void slotProvidersLoaded() )
    Q_PRIVATE_SLOT( d, void slotEntriesLoaded(const KNS3::EntryInternal::List& entries) )
    Q_PRIVATE_SLOT( d, void slotEntryChanged(const KNS3::EntryInternal& entry) )
    Q_PRIVATE_SLOT( d, void slotShowDetails(const KNS3::EntryInternal& entry) )
    Q_PRIVATE_SLOT( d, void slotShowOverview() )

    Q_PRIVATE_SLOT( d, void slotPayloadFailed(const EntryInternal& entry) )
    Q_PRIVATE_SLOT( d, void slotPayloadLoaded(QUrl url) )

    Q_PRIVATE_SLOT( d, void slotResetMessage() )
    Q_PRIVATE_SLOT( d, void slotNetworkTimeout() )
    Q_PRIVATE_SLOT( d, void sortingChanged() )
    Q_PRIVATE_SLOT( d, void slotSearchTextChanged() )
    Q_PRIVATE_SLOT( d, void slotUpdateSearch() )
    Q_PRIVATE_SLOT( d, void slotCategoryChanged(int) )

    Q_PRIVATE_SLOT( d, void slotInfo(QString provider, QString server, QString version) )
    Q_PRIVATE_SLOT( d, void slotError(const QString& message) )
    Q_PRIVATE_SLOT( d, void scrollbarValueChanged(int value) )

    friend class DownloadDialog;
};

}

#endif

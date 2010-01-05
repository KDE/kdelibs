/*
    knewstuff3/ui/downloaddialog.h.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF3_UI_DOWNLOADDIALOG_H

#include <kdialog.h>

#include "knewstuff_export.h"
#include "entry.h"

namespace KNS3
{
class ItemsModel;
class ItemsViewDelegate;
class DownloadDialogPrivate;

/**
 * KNewStuff download dialog.
 *
 * The download dialog will present items to the user
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
 *   <ol><li>StandardResource: standard ressouce dir, such as <em>.kde/share/wallpapers</em>.
 *           This is what KStandardDirs::locateLocal(name) will return.</li>
 *       <li>TargetDir: a directory in the share/apps section, such as <em>.kde/share/apps/wallpapers</em>.
 *           This is what KStandardDirs::locateLocal("data", name) will return.</li>
 *   </ol>
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT DownloadDialog :public KDialog
{
    Q_OBJECT

public:
    /**
     * Create a DownloadDialog that lets the user install, update and uninstall
     * contents. It will try to find a appname.knsrc file with the configuration.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     */
    explicit DownloadDialog(QWidget * parent = 0);

    /**
     * Create a DownloadDialog that lets the user install, update and uninstall
     * contents. Manually specify the name of a .knsrc file where the
     * KHotNewStuff configuration can be found.
     *
     * @param configFile the name of the configuration file
     * @param parent the parent of the dialog
     */
    explicit DownloadDialog(const QString& configFile, QWidget * parent = 0);

    /**
     * destructor
     */
    ~DownloadDialog();

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

    DownloadDialogPrivate *const d;
};

}

#endif

/* This file is part of the KDE project
   Copyright (C) 1998-2009 David Faure <faure@kde.org>
                 2003      Sven Leiber <s.leiber@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 or at your option version 3.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNEWFILEMENU_H
#define KNEWFILEMENU_H

#include <kactionmenu.h>
#include <kurl.h>
#include <kfile_export.h>

class KJob;

class KActionCollection;
class KNewFileMenuPrivate;

/**
 * The 'Create New' submenu, for creating files using templates
 * (e.g. "new HTML file") and directories.
 *
 * The same instance can be used by both for the File menu and the RMB popup menu,
 * in a file manager. This is also used in the file dialog's RMB menu.
 *
 * To use this class, you need to connect aboutToShow() of the File menu
 * with slotCheckUpToDate() and to call slotCheckUpToDate() before showing
 * the RMB popupmenu.
 *
 * KNewFileMenu automatically updates the list of templates shown if installed templates
 * are added/updated/deleted.
 *
 * @author Björn Ruberg <bjoern@ruberg-wegener.de>
 * Made dialogs working asynchronously
 * @author David Faure <faure@kde.org>
 * Ideas and code for the new template handling mechanism ('link' desktop files)
 * from Christoph Pickart <pickart@iam.uni-bonn.de>
 * @since 4.5
 */
class KFILE_EXPORT KNewFileMenu : public KActionMenu
{
  Q_OBJECT
public:
    /**
     * Constructor.
     * @param collection the KActionCollection this KAction should be added to.
     * @param name action name, when adding the action to the collection
     * @param parent the parent object, for ownership.
     * If the parent object is a widget, it will also used as parent widget
     * for any dialogs that this class might show. Otherwise, call setParentWidget.
     */
    KNewFileMenu(KActionCollection* collection, const QString& name, QObject* parent);

    /**
     * Destructor.
     * KNewMenu uses internally a globally shared cache, so that multiple instances
     * of it don't need to parse the installed templates multiple times. Therefore
     * you can safely create and delete KNewMenu instances without a performance issue.
     */
    virtual ~KNewFileMenu();

    /**
     * Returns the modality of dialogs
     */
    bool isModal() const;

    /**
     * Returns the files that the popup is shown for
     */
    QList<QUrl> popupFiles() const;

    /**
     * Sets the modality of dialogs created by KNewFile. Set to false if you do not want to block
     * your application window when entering a new directory name i.e.
     */
    void setModal(bool modality);

    /**
     * Sets a parent widget for the dialogs shown by KNewFileMenu.
     * This is strongly recommended, for apps with a main window.
     */
    void setParentWidget(QWidget* parentWidget);

    /**
     * Set the files the popup is shown for
     * Call this before showing up the menu
     */
    void setPopupFiles(const QList<QUrl>& files);

#ifndef KDE_NO_DEPRECATED
    void setPopupFiles(const QUrl& file)
    {
        setPopupFiles(QList<QUrl>() << file);
    }
#endif

    /**
     * Only show the files in a given set of mimetypes.
     * This is useful in specialized applications (while file managers, on
     * the other hand, want to show all mimetypes).
     */
    void setSupportedMimeTypes(const QStringList& mime);

    /**
     * Set if the directory view currently shows dot files.
     */
    void setViewShowsHiddenFiles(bool b);

    /**
     * Returns the mimetypes set in supportedMimeTypes()
     */
    QStringList supportedMimeTypes() const;

public Q_SLOTS:
    /**
     * Checks if updating the list is necessary
     * IMPORTANT : Call this in the slot for aboutToShow.
     * And while you're there, you probably want to call setViewShowsHiddenFiles ;)
     */
    void checkUpToDate();

    /**
     * Call this to create a new directory as if the user had done it using
     * a popupmenu. This is useful to make sure that creating a directory with
     * a key shortcut (e.g. F10) triggers the exact same code as when using
     * the New menu.
     * Requirements: call setPopupFiles first, and keep this KNewFileMenu instance
     * alive (the mkdir is async).
     */
    void createDirectory();

Q_SIGNALS:
    /**
     * Emitted once the file (or symlink) @p url has been successfully created
     */
    void fileCreated(const KUrl& url);

    /**
     * Emitted once the directory @p url has been successfully created
     */
    void directoryCreated(const KUrl& url);

protected Q_SLOTS:

    /**
     * Called when the job that copied the template has finished.
     * This method is virtual so that error handling can be reimplemented.
     * Make sure to call the base class slotResult when !job->error() though.
     */
    virtual void slotResult(KJob* job);


private:
    Q_PRIVATE_SLOT(d, void _k_slotAbortDialog())
    Q_PRIVATE_SLOT(d, void _k_slotActionTriggered(QAction*))
    Q_PRIVATE_SLOT(d, void _k_slotCreateDirectory(bool writeHiddenDir = false))
    Q_PRIVATE_SLOT(d, void _k_slotCreateHiddenDirectory())
    Q_PRIVATE_SLOT(d, void _k_slotFillTemplates())
    Q_PRIVATE_SLOT(d, void _k_slotOtherDesktopFile())
    Q_PRIVATE_SLOT(d, void _k_slotRealFileOrDir())
    Q_PRIVATE_SLOT(d, void _k_slotTextChanged(const QString))
    Q_PRIVATE_SLOT(d, void _k_slotSymLink())
    Q_PRIVATE_SLOT(d, void _k_slotUrlDesktopFile())

    KNewFileMenuPrivate* const d;

};

#endif

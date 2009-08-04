/*
  Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>
  Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>
  Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KDIRSELECTDIALOG_H
#define KDIRSELECTDIALOG_H

#include <kfile_export.h>

#include <kdialog.h>
#include <kurl.h>

class QAbstractItemView;

/**
 * A pretty dialog for a KDirSelect control for selecting directories.
 * @author Michael Jarrett <michaelj@corel.com>
 * @see KFileDialog
 */
class KFILE_EXPORT KDirSelectDialog : public KDialog
{
    Q_OBJECT

public:
    /**
     * Creates a new directory selection dialog.
     * @internal use the static selectDirectory function
     * @param startDir the directory, initially shown
     * @param localOnly unused. You can only select paths below the startDir
     * @param parent the parent for the dialog, usually 0L
     */
    explicit KDirSelectDialog(const KUrl& startDir = KUrl(),
                              bool localOnly = false,
                              QWidget *parent = 0L);

    /**
     * Destroys the directory selection dialog.
     */
    ~KDirSelectDialog();

    /**
     * Returns the currently selected URL, or an empty one if no item is selected.
     *
     * If the URL entered in the combobox is valid and exists, it is returned.
     * Otherwise, the URL selected in the treeview is returned instead.
     */
    KUrl url() const;

    /**
     * Returns a pointer to the view which is used for displaying the directories.
     */
    QAbstractItemView* view() const;

    /**
     * Returns whether only local directories can be selected.
     */
    bool localOnly() const;

    /**
     * Creates a KDirSelectDialog, and returns the result.
     * @param startDir the directory, initially shown
     * The tree will display this directory and subdirectories of it.
     * @param localOnly unused. You can only select paths below the startDir
     * @param parent the parent widget to use for the dialog, or NULL to create a parent-less dialog
     * @param caption the caption to use for the dialog, or QString() for the default caption
     * @return The URL selected, or an empty URL if the user canceled
     * or no URL was selected.
     *
     * NOTE: if you use this method and nothing else from libkfile,
     * then you can use KFileDialog::getExistingDirectory (if localOnly was true)
     * or KFileDialog::getExistingDirectoryUrl (if localOnly was false),
     * and then you can link to libkio only instead of libkfile.
     */
    static KUrl selectDirectory( const KUrl& startDir = KUrl(),
                                 bool localOnly = false, QWidget *parent = 0L,
                                 const QString& caption = QString());

    /**
     * @return The path for the root node
     */
    KUrl startDir() const;

public Q_SLOTS:
    /**
     * Sets the current @p url in the dialog.
     */
    void setCurrentUrl( const KUrl& url );

protected:
    virtual void accept();

    /**
     * Reimplemented for saving the dialog geometry.
     */
    virtual void hideEvent( QHideEvent *event );

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void slotCurrentChanged() )
    Q_PRIVATE_SLOT( d, void slotExpand(const QModelIndex&) )
    Q_PRIVATE_SLOT( d, void slotUrlActivated(const QString&) )
    Q_PRIVATE_SLOT( d, void slotComboTextChanged(const QString&) )
    Q_PRIVATE_SLOT( d, void slotContextMenuRequested(const QPoint&) )
    Q_PRIVATE_SLOT( d, void slotNewFolder() )
    Q_PRIVATE_SLOT( d, void slotMoveToTrash() )
    Q_PRIVATE_SLOT( d, void slotDelete() )
    Q_PRIVATE_SLOT( d, void slotProperties() )
};

#endif

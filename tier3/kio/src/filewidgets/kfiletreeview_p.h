/*
   This file is part of the KDE project

   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>

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

#ifndef KFILETREEVIEW_H
#define KFILETREEVIEW_H

#include <QTreeView>

#include <QUrl>

#include <kio/kiofilewidgets_export.h>

/**
 * The file treeview offers a treeview on the filesystem.
 */
class KIOFILEWIDGETS_EXPORT KFileTreeView : public QTreeView // exported only for kfiletreeviewtest
{
    Q_OBJECT

    public:
        /**
         * Creates a new file tree view.
         */
        KFileTreeView(QWidget *parent = 0);

        /**
         * Destroys the file tree view.
         */
        ~KFileTreeView();

        /**
         * Returns the current url.
         */
        QUrl currentUrl() const;

        /**
         * Returns the selected url.
         */
        QUrl selectedUrl() const;

        /**
         * Returns all selected urls.
         */
        QList<QUrl> selectedUrls() const;

        /**
         * Returns the current root url of the view.
         */
        QUrl rootUrl() const;

        /**
         * Returns true if the view is currently showing hidden files
         * @since 4.3
         */
        bool showHiddenFiles() const;

    /**
     * @reimplemented
     */
    QSize sizeHint() const;

    public Q_SLOTS:
        /**
         * Sets whether the dir-only mode is @p enabled.
         *
         * In dir-only mode, only directories and subdirectories
         * are listed in the view.
         */
        void setDirOnlyMode(bool enabled);

        /**
         * Sets whether hidden files shall be listed.
         */
        void setShowHiddenFiles(bool enabled);

        /**
         * Sets the current @p url of the view.
         */
        void setCurrentUrl(const QUrl &url);

        /**
         * Sets the root @p url of the view.
         *
         * The default is file:///.
         */
        void setRootUrl(const QUrl &url);

    Q_SIGNALS:
        /**
         * This signal is emitted whenever an @p url has been activated.
         */
        void activated(const QUrl &url);

        /**
         * This signal is emitted whenever the current @p url has been changed.
         */
        void currentChanged(const QUrl &url);

    protected:
        using QTreeView::currentChanged;
        virtual void contextMenuEvent( QContextMenuEvent* );

    private:
      class Private;
      Private* const d;

      Q_PRIVATE_SLOT(d, void _k_activated(const QModelIndex&))
      Q_PRIVATE_SLOT(d, void _k_currentChanged(const QModelIndex&, const QModelIndex&))
      Q_PRIVATE_SLOT(d, void _k_expanded(const QModelIndex&))
};

#endif

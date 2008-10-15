/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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
#ifndef KDIRMODEL_H
#define KDIRMODEL_H

#include <QtCore/QAbstractItemModel>
#include <kio/kio_export.h>
#include <kfileitem.h>

class KDirLister;
class KDirModelPrivate;

/**
 * @short A model for a KIO-based directory tree.
 *
 * KDirModel implements the QAbstractItemModel interface (for use with Qt's model/view widgets)
 * around the directory listing for one directory or a tree of directories.
 *
 * @see KDirSortFilterProxyModel
 *
 * @author David Faure
 * Based on work by Hamish Rodda and Pascal Letourneau
 */
class KIO_EXPORT KDirModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * @param parent parent qobject
     */
    explicit KDirModel( QObject* parent = 0 );
    ~KDirModel();

    /**
     * Set the directory lister to use by this model, instead of the default KDirLister created internally.
     * The model takes ownership.
     */
    void setDirLister( KDirLister* dirLister );

    /**
     * Return the directory lister used by this model.
     */
    KDirLister* dirLister() const;

    /**
     * Return the fileitem for a given index. This is O(1), i.e. fast.
     */
    KFileItem itemForIndex( const QModelIndex& index ) const;

    /**
     * Return the index for a given kfileitem. This can be slow.
     * @deprecated use the method that takes a KFileItem by value
     */
    KDE_DEPRECATED QModelIndex indexForItem( const KFileItem* ) const;

    /**
     * Return the index for a given kfileitem. This can be slow.
     */
    QModelIndex indexForItem( const KFileItem& ) const;

    /**
     * Return the index for a given url. This can be slow.
     */
    QModelIndex indexForUrl(const KUrl& url) const;

    /**
     * @short Lists subdirectories using fetchMore() as needed until the given @p url exists in the model.
     *
     * When the model is used by a treeview, call KDirLister::openUrl with the base url of the tree,
     * then the treeview will take care of calling fetchMore() when the user opens directories.
     * However if you want the tree to show a given URL (i.e. open the tree recursively until that URL),
     * call expandToUrl().
     * Note that this is asynchronous; the necessary listing of subdirectories will take time so
     * the model will not immediately have this url available.
     * The model emits the signal expand() when an index has become available; this can be connected
     * to the treeview in order to let it open that index.
     * @param url the url of a subdirectory of the directory model
     */
    void expandToUrl(const KUrl& url);

    /**
     * Notify the model that the item at this index has changed.
     * For instance because KMimeTypeResolver called determineMimeType on it.
     * This makes the model emit its dataChanged signal at this index, so that views repaint.
     * Note that for most things (renaming, changing size etc.), KDirLister's signals tell the model already.
     */
    void itemChanged( const QModelIndex& index );

    /***
     * Useful "default" columns. Views can use a proxy to have more control over this.
     */
    enum ModelColumns {
        Name = 0,
        Size,
        ModifiedTime,
        Permissions,
        Owner,
        Group,
        Type,
        ColumnCount
    };

    /// Possible return value for data(ChildCountRole), meaning the item isn't a directory,
    /// or we haven't calculated its child count yet
    enum { ChildCountUnknown = -1 };

    enum AdditionalRoles {
        // Note: use   printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        FileItemRole = 0x07A263FF,  ///< returns the KFileItem for a given index
        ChildCountRole = 0x2C4D0A40 ///< returns the number of items in a directory, or ChildCountUnknown
    };

    enum DropsAllowedFlag {
        NoDrops = 0,
        DropOnDirectory = 1, ///< allow drops on any directory
        DropOnAnyFile = 2, ///< allow drops on any file
        DropOnLocalExecutable = 4 ///< allow drops on local executables, shell scripts and desktop files. Can be used with DropOnDirectory.
    };
    Q_DECLARE_FLAGS(DropsAllowed, DropsAllowedFlag)

    /// Set whether dropping onto items should be allowed, and for which kind of item
    /// Drops are disabled by default.
    void setDropsAllowed(DropsAllowed dropsAllowed);

    /// Reimplemented from QAbstractItemModel. Returns true for empty directories.
    virtual bool canFetchMore ( const QModelIndex & parent ) const;
    /// Reimplemented from QAbstractItemModel. Returns ColumnCount.
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    /// Reimplemented from QAbstractItemModel.
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    /// Reimplemented from QAbstractItemModel. Not implemented yet.
    virtual bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
    /// Reimplemented from QAbstractItemModel. Lists the subdirectory.
    virtual void fetchMore ( const QModelIndex & parent );
    /// Reimplemented from QAbstractItemModel.
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    /// Reimplemented from QAbstractItemModel. Returns true for directories.
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    /// Reimplemented from QAbstractItemModel. Returns the column titles.
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    /// Reimplemented from QAbstractItemModel. O(1)
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    /// Reimplemented from QAbstractItemModel.
    virtual QMimeData * mimeData ( const QModelIndexList & indexes ) const;
    /// Reimplemented from QAbstractItemModel.
    virtual QStringList mimeTypes () const;
    /// Reimplemented from QAbstractItemModel.
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    /// Reimplemented from QAbstractItemModel.
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    /// Reimplemented from QAbstractItemModel.
    /// Call this to set a new icon, e.g. a preview
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
    /// Reimplemented from QAbstractItemModel. Not implemented. @see KDirSortFilterProxyModel
    virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

Q_SIGNALS:
    /**
     * Emitted for each subdirectory that is a parent of a url passed to expandToUrl
     * This allows to asynchronously open a tree view down to a given directory.
     */
    void expand(const QModelIndex& index);

private:
    // Make those private, they shouldn't be called by applications
    virtual bool insertRows(int , int, const QModelIndex & = QModelIndex());
    virtual bool insertColumns(int, int, const QModelIndex & = QModelIndex());
    virtual bool removeRows(int, int, const QModelIndex & = QModelIndex());
    virtual bool removeColumns(int, int, const QModelIndex & = QModelIndex());

private:
    friend class KDirModelPrivate;
    KDirModelPrivate *const d;

    Q_PRIVATE_SLOT( d, void _k_slotNewItems(const KFileItemList&) )
    Q_PRIVATE_SLOT( d, void _k_slotDeleteItems(const KFileItemList&) )
    Q_PRIVATE_SLOT( d, void _k_slotRefreshItems(const QList<QPair<KFileItem, KFileItem> >&) )
    Q_PRIVATE_SLOT( d, void _k_slotClear() )
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDirModel::DropsAllowed)

#endif /* KDIRMODEL_H */

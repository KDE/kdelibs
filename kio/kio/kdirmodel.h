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

#include <QAbstractItemModel>
#include <kurl.h>

class KFileItemList;
class KFileItem;
class KDirLister;
class KDirModelPrivate;

/**
 * @short A model for a KIO-based directory tree.
 *
 * KDirModel implements the QAbstractItemModel interface (for use with Qt's model/view widgets)
 * around the directory listing for one directory or a tree of directories.
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
    KFileItem* itemForIndex( const QModelIndex& index ) const;

    /**
     * Return the index for a given kfileitem. This can be slow.
     */
    QModelIndex indexForItem( const KFileItem* ) const;

    /**
     * Notify the model that an item has changed.
     * For instance because KMimeTypeResolver called determineMimeType on it.
     * This makes the model emit its dataChanged signal at the index for this item.
     * Note that for most things (renaming, changing size etc.), KDirLister's signals tell the model already.
     */
    void itemChanged( const KFileItem& item );

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
        ColumnCount
    };

    enum AdditionalRoles {
        // Note: use   printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        FileItemRole = 0x07A263FF  ///< returns the KFileItem* for a given index
    };

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
    /// Reimplemented from QAbstractItemModel. Not implemented.
    virtual void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );

private slots:
    void slotNewItems( const KFileItemList& );
    void slotDeleteItem( KFileItem *item );
    void slotRefreshItems( const KFileItemList& );
    void slotClear();

private:
    // Make those private, they shouldn't be called by applications
    virtual bool insertRows(int , int, const QModelIndex & = QModelIndex()) { return false; }
    virtual bool insertColumns(int, int, const QModelIndex & = QModelIndex()) { return false; }
    virtual bool removeRows(int, int, const QModelIndex & = QModelIndex()) { return false; }
    virtual bool removeColumns(int, int, const QModelIndex & = QModelIndex()) { return false; }

private:
    friend class KDirModelPrivate;
    KDirModelPrivate *const d;
};

Q_DECLARE_METATYPE(KFileItem*)

#endif /* KDIRMODEL_H */


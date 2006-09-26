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
class KDirModel : public QAbstractItemModel
{
    Q_OBJECT

public:
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
     * Return the fileitem for a given index.
     */
    KFileItem* itemForIndex( const QModelIndex& index ) const;

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

    //enum AdditionalRoles {
    //};

    virtual bool canFetchMore ( const QModelIndex & parent ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
    virtual void fetchMore ( const QModelIndex & parent );
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QMimeData * mimeData ( const QModelIndexList & indexes ) const;
    virtual QStringList mimeTypes () const;
    virtual QModelIndex parent ( const QModelIndex & index ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
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


#endif /* KDIRMODEL_H */


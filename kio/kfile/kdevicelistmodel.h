/*  This file is part of the KDE project
    Copyright (C) 2006 Michael Larouche <michael.larouche@kdemail.net>
                  2007 Kevin Ottens <ervin@kde.org>

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
#ifndef KDEVICELISTMODEL_H
#define KDEVICELISTMODEL_H

#include <kio/kio_export.h>

#include <QtCore/QAbstractItemModel>

#include <solid/device.h>
#include <solid/predicate.h>

/**
 * @brief Device list model in Qt's Interview framework.
 *
 * This class is a tree view model. Each device has a parent
 * excluding the main device(the computer). Only revelant when
 * used with QTreeView.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class KIO_EXPORT KDeviceListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit KDeviceListModel(QObject *parent=0);
    explicit KDeviceListModel(const QString &predicate, QObject *parent=0);
    explicit KDeviceListModel(const Solid::Predicate &predicate,
                              QObject *parent=0);
    ~KDeviceListModel();

    /**
     * @brief Get a visible data based on Qt role for the given index.
     * Return the device information for the give index.
     *
     * @param index The QModelIndex which contains the row, column to fetch the data.
     * @param role The Interview data role(ex: Qt::DisplayRole).
     *
     * @return the data for the given index and role.
     */
    QVariant data(const QModelIndex &index, int role) const;

    /**
     * @brief Get the header data for a given section, orientation and role.
     * This method return a value to display in header in a view.
     * Only support Qt::Horizontal direction and Qt::DisplayRole role.
     *
     * @param section Section of Header to get the data of.
     * @param orientation Orientation of the header.
     * @param role The Interview data role(ex: Qt::DisplayRole).
     *
     * @return the header data for the given section.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /**
     * @brief Get the children model index for the given row and column.
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex rootIndex() const;

    /**
     * @brief Get the parent QModelIndex for the given model child.
     */
    QModelIndex parent(const QModelIndex &child) const;

    /**
     * @brief Get the number of rows for a model index.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * @brief Get the number of columns for a model index.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Solid::Device deviceForIndex( const QModelIndex& index ) const;


Q_SIGNALS:
    void modelInitialized();

private:
    Q_PRIVATE_SLOT(d, void _k_initDeviceList())
    Q_PRIVATE_SLOT(d, void _k_deviceAdded(const QString &))
    Q_PRIVATE_SLOT(d, void _k_deviceRemoved(const QString &))

    class Private;
    Private * const d;
};

#endif

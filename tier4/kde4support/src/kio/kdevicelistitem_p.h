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
#ifndef KDEVICELISTITEM_P_H
#define KDEVICELISTITEM_P_H

#include <QVariant>
#include <QList>

namespace Solid
{
    class Device;
}

/**
 * @brief Item for the Device List model
 * Represent a Solid::Device in the tree device list model.
 * It helps to map the device in the tree.
 *
 * Keep a reference of the device that it maps.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
 */
class KDeviceListItem
{
public:
    /**
     * @brief Create a new DeviceListItem
     */
    KDeviceListItem();

    /**
     * The d-tor duh.
     */
    ~KDeviceListItem();

    /**
     * @brief Return a child of this item according to the given row.
     * @param row row of the child to retrieve.
     * @return the child DeviceListItem
     */
    KDeviceListItem *child(int row);

    /**
     * @brief Get all the children of this item.
     * @return list of children.
     */
    QList<KDeviceListItem*> children();

    /**
     * @brief Returns the index position of a child in the list.
     * @param child the child to insert.
     * @return the index position of a child in the list.
     */
    int indexOf(KDeviceListItem *child) const;

    /**
     * @brief Helper method to get the numbers of childrens of this item.
     * @return the child count of this item.
     */
    int childCount() const;

    /**
     * Get the relative(to parent) row position of this item in the tree.
     * @return the row position.
     */
    int row() const;

    /**
     * @brief Set the parent of this item
     * @param parent the parent DeviceListItem.
     */
    void setParent(KDeviceListItem *parent);

    /**
     * @brief Get the parent of this item
     * @return the parent DeviceListItem.
     */
    KDeviceListItem *parent();

    /**
     * Get the Solid::Device reference for this item.
     * @param device the Solid::Device reference.
     */
    void setDevice(const Solid::Device &device);

    /**
     * Get the Solid::Device reference for this item.
     * @return the Solid::Device reference.
     */
    Solid::Device &device();

private:
    /**
     * @brief Helper method to append a child DeviceListItem.
     * @param child child item to append.
     */
    void appendChild(KDeviceListItem *child);

    /**
     * @brief Helper method to remove a child DeviceListItem.
     * @param child child item to remove.
     */
    void removeChild(KDeviceListItem *child);

    class Private;
    Private * const d;
};

#endif

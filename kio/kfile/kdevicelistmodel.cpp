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
#include "kdevicelistmodel.h"
#include "kdevicelistitem_p.h"

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>

#include <QTimer>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

class KDeviceListModel::Private
{
public:
    Private(KDeviceListModel *self) : q(self), rootItem(new KDeviceListItem()) {}
    ~Private() { delete rootItem; }


    KDeviceListModel *q;

    KDeviceListItem *rootItem;
    QMap<QString, KDeviceListItem*> deviceItems;
    Solid::Predicate predicate;


    void initialize(const Solid::Predicate &p);
    QModelIndex indexForItem(KDeviceListItem *item) const;
    void addDevice(const Solid::Device &device);
    void removeBranch(const QString &udi);

    // Private slots
    void _k_initDeviceList();
    void _k_deviceAdded(const QString &udi);
    void _k_deviceRemoved(const QString &udi);
};

KDeviceListModel::KDeviceListModel(QObject *parent)
    : QAbstractItemModel(parent), d(new Private(this))
{
    d->deviceItems[QString()] = d->rootItem;
    d->initialize(Solid::Predicate());
}

KDeviceListModel::KDeviceListModel(const QString &predicate, QObject *parent)
    : QAbstractItemModel(parent), d(new Private(this))
{
    d->initialize(Solid::Predicate::fromString(predicate));
}

KDeviceListModel::KDeviceListModel(const Solid::Predicate &predicate, QObject *parent)
    : QAbstractItemModel(parent), d(new Private(this))
{
    d->initialize(predicate);
}

KDeviceListModel::~KDeviceListModel()
{
    delete d;
}

void KDeviceListModel::Private::initialize(const Solid::Predicate &p)
{
    predicate = p;

    // Delay load of hardware list when the event loop start
    QTimer::singleShot( 0, q, SLOT(_k_initDeviceList()) );
}

QVariant KDeviceListModel::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() )
        return QVariant();

    KDeviceListItem *deviceItem = static_cast<KDeviceListItem*>(index.internalPointer());
    Solid::Device device = deviceItem->device();

    QVariant returnData;
    if (role == Qt::DisplayRole) {
        returnData = device.product();
    }
    // Only display icons in the first column
    else if (role == Qt::DecorationRole && index.column() == 0) {
        returnData = KIcon(device.icon());
    }

    return returnData;
}

QVariant KDeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return i18n("Device name");
    }

    return QVariant();
}

QModelIndex KDeviceListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row<0 || column!=0)
        return QModelIndex();

    KDeviceListItem *parentItem;
    if (parent.isValid()) {
        parentItem = static_cast<KDeviceListItem*>(parent.internalPointer());
    } else {
        parentItem = d->rootItem;
    }

    KDeviceListItem *childItem = parentItem->child(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex KDeviceListModel::rootIndex() const
{
    return d->indexForItem(d->rootItem);
}

QModelIndex KDeviceListModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    KDeviceListItem *childItem = static_cast<KDeviceListItem*>(child.internalPointer());
    KDeviceListItem *parentItem = childItem->parent();

    if (!parentItem)
        return QModelIndex();
    else
        return d->indexForItem(parentItem);
}

int KDeviceListModel::rowCount(const QModelIndex &parent) const
{
    if( !parent.isValid() )
        return d->rootItem->childCount();

    KDeviceListItem *item = static_cast<KDeviceListItem*>(parent.internalPointer());

    return item->childCount();
}

int KDeviceListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    // We only know 1 information for a particualiar device.
    return 1;
}

Solid::Device KDeviceListModel::deviceForIndex(const QModelIndex& index) const
{
    KDeviceListItem *deviceItem = static_cast<KDeviceListItem*>(index.internalPointer());
    return deviceItem->device();
}

void KDeviceListModel::Private::_k_initDeviceList()
{
    Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();

    connect(notifier, SIGNAL(deviceAdded(const QString&)),
            q, SLOT(_k_deviceAdded(const QString&)));
    connect(notifier, SIGNAL(deviceRemoved(const QString&)),
            q, SLOT(_k_deviceRemoved(const QString&)));

    // Use allDevices() from the manager if the predicate is not valid
    // otherwise the returned list is empty
    const QList<Solid::Device> &deviceList = predicate.isValid()?
                                             Solid::Device::listFromQuery(predicate)
                                           : Solid::Device::allDevices();

    foreach(const Solid::Device &device, deviceList)
    {
        addDevice(device);
    }

    emit q->modelInitialized();
}

void KDeviceListModel::Private::addDevice(const Solid::Device &device)
{
    // Don't insert invalid devices
    if (!device.isValid()) return;

    // Don't insert devices that doesn't match the predicate set
    // (except for the root)
    if (!device.parentUdi().isEmpty()
        && predicate.isValid() && !predicate.matches(device)) {
        return;
    }

    KDeviceListItem *item;
    if (deviceItems.contains(device.udi())) { // It was already inserted as a parent
        item = deviceItems[device.udi()];
    } else {
        item = new KDeviceListItem();
        deviceItems[device.udi()] = item;
    }
    item->setDevice(device);

    KDeviceListItem *parent = rootItem;

    if (!deviceItems.contains(device.parentUdi()) ) // The parent was not present, try to insert it in the model
        addDevice( Solid::Device(device.parentUdi()) );

    if (deviceItems.contains(device.parentUdi())) // Update the parent if the device is now present
        parent = deviceItems[device.parentUdi()];

    if (item->parent()!=parent) { // If it's already our parent no need to signal the new row
        q->beginInsertRows(indexForItem(parent), parent->childCount(), parent->childCount());
        item->setParent(parent);
        q->endInsertRows();
    }
}

void KDeviceListModel::Private::removeBranch(const QString &udi)
{
    if (!deviceItems.contains(udi)) return;

    KDeviceListItem *item = deviceItems[udi];
    KDeviceListItem *parent = item->parent();

    QList<KDeviceListItem*> children = item->children();

    foreach(KDeviceListItem *child, children) {
        removeBranch(child->device().udi());
    }

    q->beginRemoveRows(indexForItem(parent),
                       item->row(), item->row());

    item->setParent(0);
    deviceItems.remove(udi);
    delete item;

    q->endRemoveRows();
}

void KDeviceListModel::Private::_k_deviceAdded(const QString &udi)
{
    Solid::Device device(udi);
    addDevice(device);
}

void KDeviceListModel::Private::_k_deviceRemoved(const QString &udi)
{
    removeBranch(udi);
}

QModelIndex KDeviceListModel::Private::indexForItem(KDeviceListItem *item) const
{
    if (item==rootItem) {
        return QModelIndex();
    } else {
        return q->createIndex(item->row(), 0, item);
    }
}

#include "kdevicelistmodel.moc"

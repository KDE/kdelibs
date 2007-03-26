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
#include "kdevicelistitem_p.h"

#include <solid/device.h>

class KDeviceListItem::Private
{
public:
    Private() : parent(0) {}

    ~Private()
    {
        qDeleteAll(children);
    }

    Solid::Device device;
    KDeviceListItem *parent;

    QList<KDeviceListItem*> children;
};

KDeviceListItem::KDeviceListItem()
    : d(new Private)
{
}

KDeviceListItem::~KDeviceListItem()
{
    delete d;
}

KDeviceListItem *KDeviceListItem::child(int row)
{
    return d->children.value(row);
}

QList<KDeviceListItem*> KDeviceListItem::children()
{
    return d->children;
}

int KDeviceListItem::indexOf(KDeviceListItem *child) const
{
    return d->children.indexOf(child);
}

int KDeviceListItem::childCount() const
{
    return d->children.count();
}

int KDeviceListItem::row() const
{
    if (d->parent) {
        return d->parent->indexOf(const_cast<KDeviceListItem*>(this));
    } else {
        return 0;
    }
}

void KDeviceListItem::setParent(KDeviceListItem *parent)
{
    if (d->parent) {
        d->parent->removeChild(this);
    }

    d->parent = parent;

    if (d->parent) {
        d->parent->appendChild(this);
    }
}

KDeviceListItem *KDeviceListItem::parent()
{
    return d->parent;
}

void KDeviceListItem::setDevice(const Solid::Device &device)
{
    d->device = device;
}

Solid::Device &KDeviceListItem::device()
{
    return d->device;
}

void KDeviceListItem::appendChild(KDeviceListItem *child)
{
    d->children.append(child);
}

void KDeviceListItem::removeChild(KDeviceListItem *child)
{
    d->children.removeAll(child);
}

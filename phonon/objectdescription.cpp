/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "objectdescription.h"
#include "objectdescription_p.h"
#include <QString>
#include <QObject>
#include <QSet>
#include "factory.h"
#include <QStringList>
#include "backendinterface.h"

namespace Phonon
{

ObjectDescriptionBase::ObjectDescriptionBase(ObjectDescriptionPrivate *dd)
    : d(dd)
{
}

ObjectDescriptionBase::ObjectDescriptionBase(const ObjectDescriptionBase &rhs)
    : d(rhs.d)
{
}

ObjectDescriptionBase::~ObjectDescriptionBase()
{
}

template<ObjectDescriptionType T>
ObjectDescription<T> &ObjectDescription<T>::operator=(const ObjectDescription<T> &rhs)
{
    d = rhs.d;
    return *this;
}

bool ObjectDescriptionBase::operator==(const ObjectDescriptionBase &otherDescription) const
{
    if (!isValid()) {
        return !otherDescription.isValid();
    }
    if (!otherDescription.isValid()) {
        return false;
    }
    return *d == *otherDescription.d;
}

int ObjectDescriptionBase::index() const
{
    if (!isValid()) {
        return -1;
    }
    return d->index;
}

QString ObjectDescriptionBase::name() const
{
    if (!isValid()) {
        return QString();
    }
    return d->name;
}

QString ObjectDescriptionBase::description() const
{
    if (!isValid()) {
        return QString();
    }
    return d->description;
}

QVariant ObjectDescriptionBase::property(const char *name) const
{
    if (!isValid()) {
        return QVariant();
    }
    return d->properties.value(name);
}

QList<QByteArray> ObjectDescriptionBase::propertyNames() const
{
    if (!isValid()) {
        return QList<QByteArray>();
    }
    return d->properties.keys();
}

bool ObjectDescriptionBase::isValid() const
{
    return d.constData() != 0;
}

ObjectDescriptionBase &ObjectDescriptionBase::operator=(const ObjectDescriptionBase &rhs)
{
    d = rhs.d;
    return *this;
}

template<ObjectDescriptionType T>
ObjectDescription<T> ObjectDescription<T>::fromIndex(int index)
{
    ObjectDescription<T> ret; //isValid() == false

    QObject *b = Factory::backend();
    BackendInterface *iface = qobject_cast<BackendInterface *>(b);
    QSet<int> indexes = iface->objectDescriptionIndexes(T);
    if (indexes.contains(index)) {
        QHash<QByteArray, QVariant> properties = iface->objectDescriptionProperties(T, index);
        ret.d = new ObjectDescriptionPrivate(index, properties);
    }
    return ret;
}

template class ObjectDescription<AudioOutputDeviceType>;
template class ObjectDescription<AudioCaptureDeviceType>;
template class ObjectDescription<VideoOutputDeviceType>;
template class ObjectDescription<VideoCaptureDeviceType>;
template class ObjectDescription<AudioEffectType>;
template class ObjectDescription<VideoEffectType>;
template class ObjectDescription<AudioCodecType>;
template class ObjectDescription<VideoCodecType>;
template class ObjectDescription<ContainerFormatType>;
template class ObjectDescription<VisualizationType>;

} //namespace Phonon
// vim: sw=4 ts=4

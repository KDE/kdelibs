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

template<ObjectDescriptionType T>
ObjectDescription<T>::ObjectDescription()
    : d(new ObjectDescriptionPrivate(-1, QHash<QByteArray, QVariant>()))
{
}

template<ObjectDescriptionType T>
ObjectDescription<T>::ObjectDescription(int index, const QHash<QByteArray, QVariant>& properties)
    : d(new ObjectDescriptionPrivate(index, properties))
{
}

template<ObjectDescriptionType T>
ObjectDescription<T>::ObjectDescription( const ObjectDescription<T>& rhs )
	: d( rhs.d )
{
}

template<ObjectDescriptionType T>
ObjectDescription<T>::~ObjectDescription()
{
}

template<ObjectDescriptionType T>
ObjectDescription<T>& ObjectDescription<T>::operator=( const ObjectDescription<T>& rhs )
{
	d = rhs.d;
	return *this;
}

template<ObjectDescriptionType T>
bool ObjectDescription<T>::operator==( const ObjectDescription<T>& otherDescription ) const
{
	return *d == *otherDescription.d;
}

template<ObjectDescriptionType T>
int ObjectDescription<T>::index() const
{
	return d->index;
}

template<ObjectDescriptionType T>
const QString& ObjectDescription<T>::name() const
{
	return d->name;
}

template<ObjectDescriptionType T>
const QString& ObjectDescription<T>::description() const
{
	return d->description;
}

template<ObjectDescriptionType T>
QVariant ObjectDescription<T>::property(const char *name) const
{
    return d->properties.value(name);
}

template<ObjectDescriptionType T>
QList<QByteArray> ObjectDescription<T>::propertyNames() const
{
    return d->properties.keys();
}

template<ObjectDescriptionType T>
bool ObjectDescription<T>::isValid() const
{
	return d->index != -1;
}

template<ObjectDescriptionType T>
ObjectDescription<T> ObjectDescription<T>::fromIndex( int index )
{
    QObject* b = Factory::backend();
    BackendInterface *iface = qobject_cast<BackendInterface*>(b);
    QSet<int> indexes = iface->objectDescriptionIndexes(T);
    if (!indexes.contains(index)) {
        return ObjectDescription<T>(); //isValid() == false
    }
    QHash<QByteArray, QVariant> properties = iface->objectDescriptionProperties(T, index);
    return ObjectDescription<T>(index, properties);
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

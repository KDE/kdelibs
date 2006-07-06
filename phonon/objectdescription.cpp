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
#include <QString>
#include <QObject>
#include <QSet>
#include "factory.h"

namespace Phonon
{

ObjectDescription::ObjectDescription()
	: d( new ObjectDescriptionPrivate( ObjectDescription::AudioOutputDevice, -1, QString(), QString() ) )
{
}

ObjectDescription::ObjectDescription( Type type, int index, const QString& name, const QString& description )
	: d( new ObjectDescriptionPrivate( type, index, name, description ) )
{
}

bool ObjectDescription::operator==( const ObjectDescription& otherDescription ) const
{
	return *d == *otherDescription.d;
}

int ObjectDescription::index() const
{
	return d->index;
}

const QString& ObjectDescription::name() const
{
	return d->name;
}

const QString& ObjectDescription::description() const
{
	return d->description;
}

bool ObjectDescription::isValid() const
{
	return d->index != -1;
}

ObjectDescription ObjectDescription::fromIndex( Type type, int index )
{
	QObject* b = Factory::self()->backend();
	QSet<int> indexes;
	QMetaObject::invokeMethod( b, "objectDescriptionIndexes", Qt::DirectConnection, Q_RETURN_ARG( QSet<int>, indexes ),
			Q_ARG( ObjectDescription::Type, type ) );
	if( !indexes.contains( index ) )
		return ObjectDescription(); //isValid() == false
	QString name, description;
	//int videoIndex;
	QMetaObject::invokeMethod( b, "objectDescriptionName", Qt::DirectConnection, Q_RETURN_ARG( QString, name ),
			Q_ARG( ObjectDescription::Type, type ), Q_ARG( int, index ) );
	QMetaObject::invokeMethod( b, "objectDescriptionDescription", Qt::DirectConnection, Q_RETURN_ARG( QString, description ),
			Q_ARG( ObjectDescription::Type, type ), Q_ARG( int, index ) );
	//QMetaObject::invokeMethod( b, "objectDescriptionVideoIndex", Qt::DirectConnection, Q_RETURN_ARG( qint32, videoIndex ),
			//Q_ARG( ObjectDescription::Type, type ), Q_ARG( int, index ) );
	return ObjectDescription( type, index, name, description );
}

} //namespace Phonon
// vim: sw=4 ts=4 noet

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

#include "objectdescriptionmodel.h"
#include "objectdescriptionmodel_p.h"
#include <QList>
#include "objectdescription.h"

#ifdef Q_D
#undef Q_D
#endif
#define Q_D(Class) Class##Private<type> * const d = d_func()

namespace Phonon
{

template<ObjectDescriptionType type>
ObjectDescriptionModel<type>::ObjectDescriptionModel( QObject* parent )
	: QAbstractListModel( parent )
	, d_ptr( new ObjectDescriptionModelPrivate<type> )
{
	d_ptr->q_ptr = this;
}

template<ObjectDescriptionType type>
ObjectDescriptionModel<type>::~ObjectDescriptionModel()
{
	delete d_ptr;
	d_ptr = 0;
}

template<ObjectDescriptionType type>
void ObjectDescriptionModel<type>::setModelData( const QList<ObjectDescription<type> >& newData )
{
	Q_D( ObjectDescriptionModel );
	d->data = newData;
	reset();
}

template<ObjectDescriptionType type>
int ObjectDescriptionModel<type>::rowCount( const QModelIndex& parent ) const
{
	if( parent.isValid() )
		return 0;

	Q_D( const ObjectDescriptionModel );
	return d->data.size();
}

template<ObjectDescriptionType type>
QVariant ObjectDescriptionModel<type>::data( const QModelIndex& index, int role ) const
{
	Q_D( const ObjectDescriptionModel );
	if( !index.isValid() || index.row() >= d->data.size() || index.column() != 0 )
		return QVariant();

	switch( role )
	{
		case Qt::EditRole:
		case Qt::DisplayRole:
			return d->data.at( index.row() ).name();
			break;
		case Qt::ToolTipRole:
			return d->data.at( index.row() ).description();
			break;
		default:
			return QVariant();
	}
}

template<ObjectDescriptionType type>
void ObjectDescriptionModel<type>::moveUp( const QModelIndex& index )
{
	Q_D( ObjectDescriptionModel );
	if( !index.isValid() || index.row() >= d->data.size() || index.row() < 1 || index.column() != 0 )
		return;

	QModelIndex above = index.sibling( index.row() - 1, index.column() );
	d->data.swap( index.row(), above.row() );
	QModelIndexList from, to;
	from << index << above;
	to << above << index;
	changePersistentIndexList( from, to );
	emit dataChanged( above, index );
}

template<ObjectDescriptionType type>
void ObjectDescriptionModel<type>::moveDown( const QModelIndex& index )
{
	Q_D( ObjectDescriptionModel );
	if( !index.isValid() || index.row() >= d->data.size() - 1 || index.column() != 0 )
		return;

	QModelIndex below = index.sibling( index.row() + 1, index.column() );
	d->data.swap( index.row(), below.row() );
	QModelIndexList from, to;
	from << index << below;
	to << below << index;
	changePersistentIndexList( from, to );
	emit dataChanged( index, below );
}

template<ObjectDescriptionType type>
QList<int> ObjectDescriptionModel<type>::tupleIndexOrder() const
{
	Q_D( const ObjectDescriptionModel );
	QList<int> ret;
	for( int i = 0; i < d->data.size(); ++i )
		ret.append( d->data.at( i ).index() );
	return ret;
}

template<ObjectDescriptionType type>
int ObjectDescriptionModel<type>::tupleIndexAtPositionIndex( int positionIndex ) const
{
	return d_func()->data.at( positionIndex ).index();
}

template class ObjectDescriptionModel<AudioOutputDeviceType>;
template class ObjectDescriptionModel<AudioCaptureDeviceType>;
template class ObjectDescriptionModel<VideoOutputDeviceType>;
template class ObjectDescriptionModel<VideoCaptureDeviceType>;
template class ObjectDescriptionModel<AudioEffectType>;
template class ObjectDescriptionModel<VideoEffectType>;
template class ObjectDescriptionModel<AudioCodecType>;
template class ObjectDescriptionModel<VideoCodecType>;
template class ObjectDescriptionModel<ContainerFormatType>;
template class ObjectDescriptionModel<VisualizationType>;

}

// vim: sw=4 ts=4 noet

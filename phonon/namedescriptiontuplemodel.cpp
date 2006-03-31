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

#include "namedescriptiontuplemodel.h"
#include "namedescriptiontuplemodel_p.h"
#include <QList>
#include "audiooutputdevice.h"
#include "audiocapturedevice.h"
#include "videocapturedevice.h"

namespace Phonon
{

NameDescriptionTupleModel::NameDescriptionTupleModel( QObject* parent )
	: QAbstractListModel( parent )
	, d_ptr( new NameDescriptionTupleModelPrivate )
{
	d_ptr->q_ptr = this;
}

NameDescriptionTupleModel::NameDescriptionTupleModel( NameDescriptionTupleModelPrivate& d, QObject* parent )
	: QAbstractListModel( parent )
	, d_ptr( &d )
{
	d_ptr->q_ptr = this;
}

void NameDescriptionTupleModel::setModelData( const QList<NameDescriptionTuple>& newData )
{
	Q_D( NameDescriptionTupleModel );
	d->data = newData;
	reset();
}

void NameDescriptionTupleModel::setModelData( const QList<AudioOutputDevice>& newData )
{
	Q_D( NameDescriptionTupleModel );
	d->data.clear();
	for( int i = 0; i < newData.size(); ++i )
		d->data.append( newData[ i ] );
	reset();
}

void NameDescriptionTupleModel::setModelData( const QList<AudioCaptureDevice>& newData )
{
	Q_D( NameDescriptionTupleModel );
	d->data.clear();
	for( int i = 0; i < newData.size(); ++i )
		d->data.append( newData[ i ] );
	reset();
}

void NameDescriptionTupleModel::setModelData( const QList<VideoCaptureDevice>& newData )
{
	Q_D( NameDescriptionTupleModel );
	d->data.clear();
	for( int i = 0; i < newData.size(); ++i )
		d->data.append( newData[ i ] );
	reset();
}

int NameDescriptionTupleModel::rowCount( const QModelIndex& parent ) const
{
	if( parent.isValid() )
		return 0;

	Q_D( const NameDescriptionTupleModel );
	return d->data.size();
}

QVariant NameDescriptionTupleModel::data( const QModelIndex& index, int role ) const
{
	Q_D( const NameDescriptionTupleModel );
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

void NameDescriptionTupleModel::moveUp( const QModelIndex& index )
{
	Q_D( NameDescriptionTupleModel );
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

void NameDescriptionTupleModel::moveDown( const QModelIndex& index )
{
	Q_D( NameDescriptionTupleModel );
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

QList<int> NameDescriptionTupleModel::tupleIndexOrder() const
{
	Q_D( const NameDescriptionTupleModel );
	QList<int> ret;
	for( int i = 0; i < d->data.size(); ++i )
		ret.append( d->data.at( i ).index() );
	return ret;
}

int NameDescriptionTupleModel::tupleIndexAtPositionIndex( int positionIndex ) const
{
	return d_func()->data.at( positionIndex ).index();
}

}

#include "namedescriptiontuplemodel.moc"
// vim: sw=4 ts=4 noet

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
#include <kicon.h>
#include <kdebug.h>
#include <QMimeData>

#ifdef Q_D
#undef Q_D
#endif
#define Q_D(Class) Class##Private<type> * const d = d_func()

#if Q_MOC_OUTPUT_REVISION != 59
#ifdef __GNUC__
#warning "Parts of this file were written to resemble the output of the moc"
#warning "from 4.2.2. Please make sure that it still works correctly with your"
#warning "version of Qt."
#endif
#endif

static const uint qt_meta_data_Phonon__ObjectDescriptionModel[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioOutputDeviceType[]  = { "Phonon::AudioOutputDevice\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioCaptureDeviceType[] = { "Phonon::AudioCaptureDevice\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoOutputDeviceType[]  = { "Phonon::VideoOutputDevice\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoCaptureDeviceType[] = { "Phonon::VideoCaptureDevice\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioEffectType[]        = { "Phonon::AudioEffectDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoEffectType[]        = { "Phonon::VideoEffectDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioCodecType[]         = { "Phonon::AudioCodecDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoCodecType[]         = { "Phonon::VideoCodecDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_ContainerFormatType[]    = { "Phonon::ContainerFormatDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VisualizationType[]      = { "Phonon::VisualizationDescription\0" };

namespace Phonon
{

template<> const QMetaObject ObjectDescriptionModel<AudioOutputDeviceType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioOutputDeviceType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<AudioCaptureDeviceType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioCaptureDeviceType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VideoOutputDeviceType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoOutputDeviceType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VideoCaptureDeviceType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoCaptureDeviceType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<AudioEffectType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioEffectType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VideoEffectType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoEffectType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<AudioCodecType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioCodecType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VideoCodecType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoCodecType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<ContainerFormatType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_ContainerFormatType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VisualizationType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VisualizationType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};

template<ObjectDescriptionType type>
const QMetaObject *ObjectDescriptionModel<type>::metaObject() const
{
    return &staticMetaObject;
}

template<ObjectDescriptionType type>
void *ObjectDescriptionModel<type>::qt_metacast(const char *_clname)
{
    if (!_clname) {
        return 0;
    }
    if (!strcmp(_clname, ObjectDescriptionModel<type>::staticMetaObject.className())) {
        return static_cast<void*>(const_cast<ObjectDescriptionModel<type>*>(this));
    }
    return QAbstractListModel::qt_metacast(_clname);
}

/*
template<ObjectDescriptionType type>
int ObjectDescriptionModel<type>::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    return QAbstractListModel::qt_metacall(_c, _id, _a);
}
*/

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
QList<ObjectDescription<type> > ObjectDescriptionModel<type>::modelData() const
{
    Q_D(const ObjectDescriptionModel);
    return d->data;
}

template<ObjectDescriptionType type>
ObjectDescription<type> ObjectDescriptionModel<type>::modelData(const QModelIndex &index) const
{
    Q_D(const ObjectDescriptionModel);
    if (!index.isValid() || index.row() >= d->data.size() || index.column() != 0) {
        return ObjectDescription<type>();
    }
    return d->data.at(index.row());
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
        case Qt::DecorationRole:
            {
                QVariant icon = d->data.at(index.row()).property("icon");
                if (icon.isValid()) {
                    if (icon.type() == QVariant::String) {
                        return KIcon(icon.toString());
                    } else if (icon.type() == QVariant::Icon) {
                        return icon;
                    }
                }
            }
            return QVariant();
		default:
			return QVariant();
	}
}

template<ObjectDescriptionType type>
Qt::ItemFlags ObjectDescriptionModel<type>::flags(const QModelIndex &index) const
{
    Q_D(const ObjectDescriptionModel);
    if(!index.isValid() || index.row() >= d->data.size() || index.column() != 0) {
        return Qt::ItemIsDropEnabled;
    }

    QVariant available = d->data.at(index.row()).property("available");
    if (available.isValid() && available.type() == QVariant::Bool && !available.toBool()) {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

template<ObjectDescriptionType type>
void ObjectDescriptionModel<type>::moveUp( const QModelIndex& index )
{
	Q_D( ObjectDescriptionModel );
	if( !index.isValid() || index.row() >= d->data.size() || index.row() < 1 || index.column() != 0 )
		return;

    emit layoutAboutToBeChanged();
	QModelIndex above = index.sibling( index.row() - 1, index.column() );
	d->data.swap( index.row(), above.row() );
	QModelIndexList from, to;
	from << index << above;
	to << above << index;
	changePersistentIndexList( from, to );
    emit layoutChanged();
}

template<ObjectDescriptionType type>
void ObjectDescriptionModel<type>::moveDown( const QModelIndex& index )
{
	Q_D( ObjectDescriptionModel );
	if( !index.isValid() || index.row() >= d->data.size() - 1 || index.column() != 0 )
		return;

    emit layoutAboutToBeChanged();
	QModelIndex below = index.sibling( index.row() + 1, index.column() );
	d->data.swap( index.row(), below.row() );
	QModelIndexList from, to;
	from << index << below;
	to << below << index;
	changePersistentIndexList( from, to );
    emit layoutChanged();
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

template<ObjectDescriptionType type>
Qt::DropActions ObjectDescriptionModel<type>::supportedDropActions() const
{
    //kDebug(600) << k_funcinfo << endl;
    return Qt::CopyAction | Qt::MoveAction;
}

template<ObjectDescriptionType type>
bool ObjectDescriptionModel<type>::dropMimeData(const QMimeData *data, Qt::DropAction action,
        int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(column);
    //kDebug(600) << k_funcinfo << data << action << row << column << parent << endl;

    QString format = mimeTypes().first();
    if (!data->hasFormat(format)) {
        return false;
    }

    Q_D(ObjectDescriptionModel);
    if (parent.isValid()) {
        row = parent.row();
    } else {
        if (row == -1) {
            row = d->data.size();
        }
    }

    QByteArray encodedData = data->data(format);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<ObjectDescription<type> > toInsert;
    while (!stream.atEnd()) {
        int otherIndex;
        stream >> otherIndex;
        ObjectDescription<type> obj = ObjectDescription<type>::fromIndex(otherIndex);

        if (obj.isValid()) {
            toInsert << obj;
        }
    }
    beginInsertRows(QModelIndex(), row, row + toInsert.size() - 1);
    foreach (const ObjectDescription<type> &obj, toInsert) {
        d->data.insert(row, obj);
    }
    endInsertRows();
    return true;
}

template<ObjectDescriptionType type>
bool ObjectDescriptionModel<type>::removeRows(int row, int count, const QModelIndex &parent)
{
    //kDebug(600) << k_funcinfo << row << count << parent << endl;
    Q_D(ObjectDescriptionModel);
    if (parent.isValid() || row + count > d->data.size()) {
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for (;count > 0; --count) {
        d->data.removeAt(row);
    }
    endRemoveRows();
    return true;
}

/*
template<ObjectDescriptionType type>
bool ObjectDescriptionModel<type>::insertRows(int row, int count, const QModelIndex &parent)
{
    kDebug(600) << k_funcinfo << row << count << parent << endl;
    Q_D(ObjectDescriptionModel);
    if (parent.isValid() || row < 0 || row > d->data.size()) {
        return false;
    }
    beginInsertRows(parent, row, row + count - 1);
    for (;count > 0; --count) {
        d->data.insert(row, ObjectDescription<type>());
    }
    endInsertRows();
    return true;
}
*/

template<ObjectDescriptionType type>
QStringList ObjectDescriptionModel<type>::mimeTypes() const
{
    return QStringList(QLatin1String("application/x-phonon-objectdescription") + QString::number(static_cast<int>(type)));
}

template<ObjectDescriptionType type>
QMimeData *ObjectDescriptionModel<type>::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const ObjectDescriptionModel);

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QModelIndexList::const_iterator end = indexes.constEnd();
    QModelIndexList::const_iterator index = indexes.constBegin();
    for(; index!=end; ++index) {
        if ((*index).isValid()) {
            ObjectDescription<type> item = d->data.at((*index).row());
            stream << item.index();
        }
    }
    //kDebug(600) << k_funcinfo << "setting mimeData to " << encodedData << endl;
    mimeData->setData(mimeTypes().first(), encodedData);
    return mimeData;
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

// vim: sw=4 ts=4

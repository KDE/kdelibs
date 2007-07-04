/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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
//#include "objectdescriptionmodel_p.h"
#include "phonondefs_p.h"
#include <QtCore/QList>
#include "objectdescription.h"
#include "phononnamespace_p.h"
#include <QtCore/QMimeData>
#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include "factory.h"

//X #ifdef USE_TROLLTECH_LABS_MODELTEST
//X #include <QtCore/QSet>
//X #include "modeltest.h"
//X #endif

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
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioStreamType[]        = { "Phonon::AudioStreamDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoStreamType[]        = { "Phonon::VideoStreamDescription\0" };
static const char qt_meta_stringdata_Phonon__ObjectDescriptionModel_SubtitleStreamType[]     = { "Phonon::SubtitleStreamDescription\0" };

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
template<> const QMetaObject ObjectDescriptionModel<AudioStreamType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_AudioStreamType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<VideoStreamType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_VideoStreamType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};
template<> const QMetaObject ObjectDescriptionModel<SubtitleStreamType>::staticMetaObject = {
    { &QAbstractListModel::staticMetaObject, qt_meta_stringdata_Phonon__ObjectDescriptionModel_SubtitleStreamType,
      qt_meta_data_Phonon__ObjectDescriptionModel, 0 }
};

QVariant ObjectDescriptionModelData::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->data.size() || index.column() != 0)
        return QVariant();

    switch(role)
    {
    case Qt::EditRole:
    case Qt::DisplayRole:
        return d->data.at(index.row())->name();
    case Qt::ToolTipRole:
        return d->data.at(index.row())->description();
    case Qt::DecorationRole:
        {
            const QVariant icon = d->data.at(index.row())->property("icon");
            if (icon.type() == QVariant::String) {
                return Factory::icon(icon.toString());
            } else if (icon.type() == QVariant::Icon) {
                return icon;
            }
        }
        // fall through
    default:
        return QVariant();
    }
}

#if 0
#define INSTANTIATE_META_FUNCTIONS(type) \
template PHONON_EXPORT const QMetaObject *ObjectDescriptionModel<type>::metaObject() const; \
template void *ObjectDescriptionModel<type>::qt_metacast(const char *)

INSTANTIATE_META_FUNCTIONS(AudioOutputDeviceType);
INSTANTIATE_META_FUNCTIONS(AudioCaptureDeviceType);
INSTANTIATE_META_FUNCTIONS(VideoOutputDeviceType);
INSTANTIATE_META_FUNCTIONS(VideoCaptureDeviceType);
INSTANTIATE_META_FUNCTIONS(AudioEffectType);
INSTANTIATE_META_FUNCTIONS(VideoEffectType);
INSTANTIATE_META_FUNCTIONS(AudioCodecType);
INSTANTIATE_META_FUNCTIONS(VideoCodecType);
INSTANTIATE_META_FUNCTIONS(ContainerFormatType);
INSTANTIATE_META_FUNCTIONS(VisualizationType);
INSTANTIATE_META_FUNCTIONS(AudioStreamType);
INSTANTIATE_META_FUNCTIONS(VideoStreamType);
INSTANTIATE_META_FUNCTIONS(SubtitleStreamType);
#endif

} // namespace Phonon

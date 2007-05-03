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

#ifndef PHONON_BACKENDINTERFACE_H
#define PHONON_BACKENDINTERFACE_H

#include "phonon_export.h"
#include "objectdescription.h"

#include <QtCore/QtGlobal>

class QVariant;

namespace Phonon
{

/**
 * \short Main Backend class interface
 *
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 */
class BackendInterface
{
    public:
        virtual ~BackendInterface() {}

        /**
         * Classes that take no additional arguments
         */
        enum Class {
            MediaObjectClass,
            AudioPathClass,
            VolumeFaderEffectClass,
            AudioOutputClass,
            AudioDataOutputClass,
            VisualizationClass,
            VideoPathClass,
            BrightnessControlClass,
            VideoDataOutputClass,
            DeinterlaceFilterClass,
            /**
             * Takes an additional int that specifies the effect Id.
             */
            AudioEffectClass,
            /**
             * Takes an additional int that specifies the effect Id.
             */
            VideoEffectClass,
            VideoWidgetClass
        };

        /**
         * Returns a new instance of the requested class.
         *
         * \param c The requested class.
         * \param parent The parent object.
         * \param arg1 An additional argument (documented in \ref Class1).
         */
        virtual QObject *createObject(Class c, QObject *parent, const QList<QVariant> &args = QList<QVariant>()) = 0;

        virtual QSet<int> objectDescriptionIndexes(ObjectDescriptionType type) const = 0;
        virtual QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const = 0;
};
} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::BackendInterface, "BackendInterface2.phonon.kde.org")

#endif // PHONON_BACKENDINTERFACE_H

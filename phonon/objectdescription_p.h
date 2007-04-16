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

#ifndef PHONON_OBJECTDESCRIPTION_P_H
#define PHONON_OBJECTDESCRIPTION_P_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>
#include <kdebug.h>

namespace Phonon
{
    class ObjectDescriptionPrivate : public QSharedData
    {
        public:
            ObjectDescriptionPrivate(int _index, const QHash<QByteArray, QVariant> &_properties)
                : index(_index),
                name(_properties["name"].toString()),
                description(_properties["description"].toString()),
                properties(_properties)
            {
            }

            bool operator==(const ObjectDescriptionPrivate &rhs) const
            {
                if (index == rhs.index && (name != rhs.name || description != rhs.description))
                    kError(600) << "Same index (" << index <<
                        "), but different name/description. This is a bug in the Phonon backend." << endl;
                return index == rhs.index;// && name == rhs.name && description == rhs.description;
            }

            int index;
            QString name, description;
            QHash<QByteArray, QVariant> properties;
    };
} // namespace Phonon

#endif // PHONON_OBJECTDESCRIPTION_P_H
// vim: sw=4 ts=4 tw=80

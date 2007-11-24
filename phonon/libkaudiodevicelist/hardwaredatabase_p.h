/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 or version 3 as published by the Free Software
    Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PHONON_HARDWAREDATABASE_P_H
#define PHONON_HARDWAREDATABASE_P_H

#include <QtCore/QString>
#include "kaudiodevicelist_export.h"

namespace Phonon
{

namespace HardwareDatabase
{
    class Entry;
    struct HardwareDatabasePrivate;

    KAUDIODEVICELIST_EXPORT bool contains(const QString &udi);
    KAUDIODEVICELIST_EXPORT Entry entryFor(const QString &udi);

    class Entry
    {
        public:
            const QString name;
            const QString iconName;
            const int initialPreference;
            const int isAdvanced;

        private:
            friend struct HardwareDatabasePrivate;
            friend Entry entryFor(const QString &);
            inline Entry(const QString &_name, const QString &_iconName, int _initialPreference, int _isAdvanced)
                : name(_name), iconName(_iconName), initialPreference(_initialPreference), isAdvanced(_isAdvanced) {}
            inline Entry() : initialPreference(0), isAdvanced(0) {}
    };
} // namespace HardwareDatabase

} // namespace Phonon
#endif // PHONON_HARDWAREDATABASE_P_H

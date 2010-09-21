/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NEPOMUK_UTILS_GLOBAL_H_
#define _NEPOMUK_UTILS_GLOBAL_H_

#include "nepomukutils_export.h"
#include "resource.h"

#include <QtCore/QFlags>
#include <QtCore/QDateTime>
#include <QtCore/QList>

#include "kurl.h"

namespace Nepomuk {
    class Variant;

    namespace Types {
        class Property;
    }

    namespace Utils {
        /**
         * Flags to change the behaviour of formatPropertyValue().
         *
         * \since 4.6
         */
        enum PropertyFormatFlag {
            /**
             * No flags, ie. default behaviour.
             */
            NoPropertyFormatFlags = 0x0,

            /**
             * Include html links to %Nepomuk resources, files, and
             * %Nepomuk queries.
             */
            WithKioLinks = 0x1
        };
        Q_DECLARE_FLAGS( PropertyFormatFlags, PropertyFormatFlag )

        /**
         * Format a property to be displayed to the user.
         *
         * \param property The actual property to which the value applies to.
         * \param value The actual value to be formatted.
         * \param flags Optional flags to change the resulting string.
         * \param resources The resources which the property applies to. Depending on the property
         * the user readable representation might differ based on the resources' types. Using an
         * empty list is perfectly valid.
         *
         * \return A user readable representation of the value which corresponds to \p resource
         * and \p property. This string may contain Qt-style HTML tags for links and special formatting.
         *
         * \since 4.6
         */
        NEPOMUKUTILS_EXPORT QString formatPropertyValue( const Nepomuk::Types::Property& property,
                                                         const Nepomuk::Variant& value,
                                                         const QList<Nepomuk::Resource>& resources = QList<Nepomuk::Resource>(),
                                                         PropertyFormatFlags flags = NoPropertyFormatFlags );

        /**
         * Save a copy event in %Nepomuk stating that \p src has been copied to \p dest
         * with an optional download \p startTime and an optional \p referrer URL that
         * indicates from which webpage a download was initiated.
         *
         * \return The download event resource. Use finishCopyEvent() to store the event's end time.
         *
         * \since 4.6
         */
        NEPOMUKUTILS_EXPORT Nepomuk::Resource createCopyEvent( const KUrl& src,
                                                               const KUrl& dest,
                                                               const QDateTime& startTime = QDateTime(),
                                                               const KUrl& referrer = KUrl() );

        /**
         * Save the end time of a copy event created via createCopyEvent().
         *
         * \param eventResource The resource returned by createCopyEvent().
         * \param endTime The time the copy event ended to the saved.
         *
         * \since 4.6
         */
        NEPOMUKUTILS_EXPORT void finishCopyEvent( Nepomuk::Resource& eventResource, const QDateTime& endTime );
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Nepomuk::Utils::PropertyFormatFlags )

#endif

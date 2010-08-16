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

#ifndef _NEPOMUK_FILE_H_
#define _NEPOMUK_FILE_H_

#include "resource.h"
#include "nepomuk_export.h"

namespace Nepomuk {
    /**
     * \class File file.h Nepomuk/File
     *
     * \brief A Nepomuk resource representing a file.
     *
     * %File is a convinience class which allows to easily handle file
     * resources which are sort of a special case in %Nepomuk.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \since 4.6
     */
    class NEPOMUK_EXPORT File : public Resource
    {
    public:
        /**
         * Create a new file resource.
         *
         * \param url The URL to the file.
         * \param manager The resource manager to use. This allows to mix resources from different
         * managers and, thus, different models.
         */
        File( const KUrl& url = KUrl(), ResourceManager* manager = 0 );

        /**
         * Construct from a Resource
         */
        File( const Resource& other );

        /**
         * Desctructor
         */
        ~File();

        /**
         * Assignment operator.
         */
        File& operator=( const KUrl& url );

        /**
         * The URL of the file. Be aware that this differs from
         * Resource::resourceUri() and is stored as nie:url
         * in the %Nepomuk database.
         *
         * \return The URL of the file or an empty KUrl in case this
         * resource does not represent a file.
         */
        KUrl url() const;

        /**
         * Returns the resource representing the containing folder,
         * ie. the folder containing this file resource.
         *
         * \return The resource representing the folder or an invalid
         * resource in case this resource is invalid or not a file.
         */
        File dirResource() const;
    };
}

#endif

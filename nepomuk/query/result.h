/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_RESULT_H_
#define _NEPOMUK_QUERY_RESULT_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QHash>

#include <Soprano/Statement>

#include "nepomukquery_export.h"

namespace Nepomuk {

    class Resource;
    namespace Types {
        class Property;
    }

    namespace Query {
        /**
         * \class Result result.h Nepomuk/Query/Result
         *
         * \brief A single search result.
         *
         * A search via QueryServiceClient returns a set of Result object. A result consists
         * of a Nepomuk::Resource and an optional score.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT Result
        {
        public:
            /**
             * Create an empty result.
             */
            Result();

            /**
             * Create a new result.
             *
             * \param resource The result resource.
             * \param score The optional result score.
             */
            Result( const Nepomuk::Resource& resource, double score = 0.0 );

            /**
             * Copy constructor.
             */
            Result( const Result& );

            /**
             * Destructor
             */
            ~Result();

            /**
             * Assignment operator
             */
            Result& operator=( const Result& );

            /**
             * The score of the result. By default the value is 0.0
             * which means no score.
             *
             * \sa setScore
             */
            double score() const;

            /**
             * The result resource.
             */
            Resource resource() const;

            /**
             * Set the score of the result.
             *
             * \sa score
             */
            void setScore( double score );

            /**
             * Add the value of a request property.
             *
             * \sa Query::RequestProperty
             */
            void addRequestProperty( const Types::Property& property, const Soprano::Node& value );

            /**
             * Retrieve the values of the request properties.
             *
             * \sa Query::RequestProperty
             */
            QHash<Types::Property, Soprano::Node> requestProperties() const;

            /**
             * Retrieve value of request property \p property.
             *
             * \sa requestProperties, addRequestProperty
             */
            Soprano::Node operator[]( const Types::Property& property ) const;

            /**
             * Retrieve value of request property \p property.
             *
             * \sa requestProperties, addRequestProperty
             */
            Soprano::Node requestProperty( const Types::Property& property ) const;

            /**
             * Comparison operator
             */
            bool operator==( const Result& ) const;

        private:
            class Private;
            QSharedDataPointer<Private> d;
        };
    }
}

#endif

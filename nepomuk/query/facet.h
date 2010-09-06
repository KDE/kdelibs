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

#ifndef _NEPOMUK_QUERY_FACET_H_
#define _NEPOMUK_QUERY_FACET_H_

#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QSharedDataPointer>

#include "term.h"

#include "nepomukquery_export.h"

class QString;

namespace Nepomuk {
    namespace Types {
        class Property;
    }

    namespace Query {
        class Query;

        class NEPOMUKQUERY_EXPORT Facet
        {
        public:
            Facet();
            Facet( const Facet& other );
            ~Facet();

            Facet& operator=( const Facet& other );

            QString title() const;
            bool exclusive() const;
            QString rangeTitle() const;
            Types::Property rangeProperty() const;

            void setTitle( const QString& title );
            void setExclusive( bool exclusive );

            /**
             * Allows to set a custom title for the first entry in the
             * facet selection. Defaults to "None". Be aware that the
             * no selection term is only used for exclusive() facets.
             */
            void setNoSelectionTitle( const QString& title );

            /**
             * Especially dates are better expressed in ranges.
             * While a group normally only contains fixed Facets
             * the range allows to provide a user selection
             * of the range in which the facet should result.
             *
             * By default the property is invalid which means that
             * no range is available.
             */
            void setRangeProperty( const Types::Property& prop );

            /**
             * Set the title displayed to the user when offering GUI
             * for the range selection.
             */
            void setRangeTitle( const QString& title );

            /**
             * Used by the GUI to set the user selected range. The corresponding
             * term can be created via createRangeTerm(). This avoids caching the
             * values in the GUI classes.
             */
            void setRange( const QVariant& start, const QVariant& end );

            /**
             * Used by the GUI to create the term for a range selected by the user.
             */
            Term createRangeTerm() const;

            void addTerm( const Term& term, const QString& title );
            void clear();

            /**
             * The number of terms in the facet. This includes the no selection term
             * (in case the facet is exclusive) and the optional range term.
             */
            int count() const;

            Term termAt( int i ) const;
            QString titleAt( int i ) const;
            QString termTitle( const Term& term ) const;

            QList<Term> termList() const;

            bool operator==( const Facet& other ) const;
            bool operator!=( const Facet& other ) const;

        private:
            class FacetPrivate;
            QSharedDataPointer<FacetPrivate> d;
        };

        NEPOMUKQUERY_EXPORT uint qHash( const Facet& facet );
    }
}

#endif

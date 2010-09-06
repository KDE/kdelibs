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

#ifndef _NEPOMUK_QUERY_FACET_CONTAINER_H_
#define _NEPOMUK_QUERY_FACET_CONTAINER_H_

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QVector>
#include <QtCore/QPointer>

#include "facet.h"
#include "query.h"

namespace Soprano {
    namespace Util {
        class AsyncQuery;
    }
}

namespace Nepomuk {
    namespace Query {
        class FacetContainer : public QObject
        {
            Q_OBJECT

        public:
            FacetContainer( const Facet& facet, QObject* parent = 0 );
            ~FacetContainer();

            Facet facet() const { return m_facet; }

            QList<int> selectedFacets() const { return m_selectedFacets.toList(); }
            bool isSelected(int index) const { return m_selectedFacets.contains(index); }

            void setSelected( int index, bool selected );

            Term selectedTerm() const;

            int frequency( int index ) const { return m_frequencies[index]; }

            /**
             * Select terms according to \p term. In case this facet is non-exclusive
             * and \p term is an OrTerm the subterms are checked.
             */
            bool selectFromTerm( const Term& term );

        public Q_SLOTS:
            void reset();
            void resetFrequencies();
            void updateFrequencies( const Nepomuk::Query::Query& baseQuery );

        Q_SIGNALS:
            void frequencyChanged( Nepomuk::Query::FacetContainer*, int index, int count );
            void selectionChanged( Nepomuk::Query::FacetContainer* );

        private Q_SLOTS:
            void slotCountQueryDone( Soprano::Util::AsyncQuery* );
            void queryNextFrequency();

        private:
            int indexOf( const Term& term ) const;

            Facet m_facet;
            QSet<int> m_selectedFacets;
            QVector<int> m_frequencies;

            // status variables for the frequency calculation
            Query m_frequencyBaseQuery;
            int m_currentFrequencyIndex;
            QPointer<Soprano::Util::AsyncQuery> m_countQuery;
        };
    }
}

#endif

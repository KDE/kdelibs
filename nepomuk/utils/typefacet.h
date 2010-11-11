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

#ifndef _NEPOMUK_TYPEFACET_H_
#define _NEPOMUK_TYPEFACET_H_

#include "facet.h"

namespace Nepomuk {
    namespace Utils {

        class TypeFacet : public Nepomuk::Utils::Facet
        {
            Q_OBJECT

        public:
            TypeFacet( QObject *parent = 0 );
            ~TypeFacet();

            SelectionMode selectionMode() const;
            Query::Term queryTerm() const;
            int count() const;
            KGuiItem guiItem( int index ) const;
            bool isSelected( int index ) const;

        public Q_SLOTS:
            void clearSelection();
            void setSelected( int index, bool selected = true );
            bool selectFromTerm( const Nepomuk::Query::Term& queryTerm );

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif

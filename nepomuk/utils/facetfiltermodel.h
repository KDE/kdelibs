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

#ifndef FACETFILTERMODEL_H
#define FACETFILTERMODEL_H

#include <QtGui/QSortFilterProxyModel>

namespace Nepomuk {
    namespace Utils {
        /**
         * \class FacetFilterModel facetfiltermodel.h Nepomuk/Utils/FacetFilterModel
         *
         * \ingroup nepomuk_facets
         *
         * \brief Filter model that hides irrelevant Facets from a FacetModel.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class FacetFilterModel : public QSortFilterProxyModel
        {
            Q_OBJECT

        public:
            FacetFilterModel(QObject *parent = 0);
            ~FacetFilterModel();

        protected:
            bool filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const;

        private:
            class Private;
            Private* const d;
        };
    }
}

#endif // FACETFILTERMODEL_H

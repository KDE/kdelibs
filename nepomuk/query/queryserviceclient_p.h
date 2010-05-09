/*
   Copyright (c) 2010 Dario Freddi <drf@kde.org>

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

#ifndef _NEPOMUK_QUERY_SERVICE_CLIENT_P_H_
#define _NEPOMUK_QUERY_SERVICE_CLIENT_P_H_

#include "result.h"

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

namespace {

    class NepomukResultListEventLoop : public QEventLoop
    {
        Q_OBJECT
        public:
            NepomukResultListEventLoop(Nepomuk::Query::QueryServiceClient *parent);
            virtual ~NepomukResultListEventLoop();

            QList<Nepomuk::Query::Result> result() const;

        public Q_SLOTS:
            void addEntries(const QList<Nepomuk::Query::Result> &entries);

        private:
            QList<Nepomuk::Query::Result> m_result;
    };

}

#endif

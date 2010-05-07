/*
   This file is part of the Nepomuk KDE project.
   Copyright 2008-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_MASS_UPDATE_JOB_H_
#define _NEPOMUK_MASS_UPDATE_JOB_H_

#include "kjob.h"
#include "kurl.h"

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QTimer>

#include "resource.h"
#include "variant.h"


namespace Nepomuk {
    /**
     * \class MassUpdateJob nepomukmassupdatejob.h Nepomuk/MassUpdateJob
     *
     * \brief A KJob to set Nepomuk properties on a set of resources at once.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     */
    class NEPOMUK_EXPORT MassUpdateJob : public KJob
    {
        Q_OBJECT

    public:
        /**
         * Default constructor. Normally there is no need to use this.
         * Use tagResources(), rateResources(), or commentResources()
         * instead.
         */
        MassUpdateJob( QObject* parent = 0 );

        /**
         * Destructor
         */
        ~MassUpdateJob();

        /**
         * Set a list of files to change
         * This has the same effect as using setResources
         * with a list of manually created resources.
         */
        void setFiles( const KUrl::List& urls );

        /**
         * Set a list of resources to change.
         */
        void setResources( const QList<Nepomuk::Resource>& );

        /**
         * Set the properties to change in the mass update.
         */
        void setProperties( const QList<QPair<QUrl, Nepomuk::Variant> >& props );

        /**
         * Actually start the job.
         */
        void start();

        static MassUpdateJob* tagResources( const QList<Nepomuk::Resource>&, const QList<Nepomuk::Tag>& tags );
        static MassUpdateJob* rateResources( const QList<Nepomuk::Resource>&, int rating );
        static MassUpdateJob* commentResources( const QList<Nepomuk::Resource>&, const QString& comment);

    protected:
        bool doKill();
        bool doSuspend();
        bool doResume();

    private Q_SLOTS:
        void slotNext();

    private:
        QList<Nepomuk::Resource> m_resources;
        QList<QPair<QUrl,Nepomuk::Variant> > m_properties;
        int m_index;
        QTimer m_processTimer;
    };
}

#endif

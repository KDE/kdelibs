/* This file is part of the KDE Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SERVICE_H_
#define _NEPOMUK_SERVICE_H_

#include <QtCore/QObject>
#include "nepomuk_export.h"

namespace Soprano {
    class Model;
}

namespace Nepomuk {
    /**
     * \class Service nepomukservice.h Nepomuk/Service
     *
     * \brief Base class for all Nepomuk services.
     *
     * A %Nepomuk service is intended to perform some kind of
     * operation on the %Nepomuk data storage. This can include
     * data gathering, data enrichment, or enhanced data query.
     *
     * %Nepomuk services are started and managed by the %Nepomuk
     * server. Very much like KDED modules a %Nepomuk service
     * has autostart and start-on-demand properties. In addition
     * a %Nepomuk service can define an arbitrary number of
     * dependencies which are necessary to run the service. These
     * dependencies name other services.
     *
     * To create a new %Nepomuk service one derives a new class from
     * Nepomuk::Service and exports it as a standard KDE module, i.e.
     * plugin.
     *
     * \code
     * class MyService : public Nepomuk::Service
     * {
     *    // do fancy stuff
     * };
     * \endcode
     *
     * Export it as a %Nepomuk service plugin:
     *
     * \code
     * NEPOMUK_EXPORT_SERVICE(MyService, "mynepomukservice")
     * \endcode
     *
     * A desktop file describes the service's properties:
     *
     * \code
     * [Desktop Entry]
     * Type=Service
     * X-KDE-ServiceTypes=NepomukService
     * X-KDE-Library=mynepomukservice
     * X-KDE-Nepomuk-autostart=true
     * X-KDE-Nepomuk-start-on-demand=false
     * # Dependencies default to 'nepomukstorage'
     * X-KDE-Nepomuk-dependencies=nepomukfoobar
     * Name=My fancy Nepomuk Service
     * Comment=A Nepomuk service that does fancy things
     * \endcode
     *
     * The %Nepomuk server will automatically export all D-Bus
     * interfaces defined on the service instance. Thus, the 
     * simplest way to export methods via D-Bus is by marking
     * them with Q_SCRIPTABLE.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \since 4.1
     */
    class NEPOMUK_EXPORT Service : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Create a new Service.
         *
         * \param parent The parent object
         * \param delayedInitialization If \p true the service will not
         * be usable until setServiceInitialized has been called.
         * This allows to design services that need to perform 
         * long initialization tasks.
         */
        Service( QObject* parent = 0, bool delayedInitialization = false );

        /**
         * Destructor
         */
        virtual ~Service();

    protected:
        /**
         * \return A wrapper model which provides
         * a connection to the %Nepomuk server.
         */
        Soprano::Model* mainModel();

        /**
         * A %Nepomuk service can make use of a warmup phase in which
         * it is not usable yet. Call this method once your service
         * is fully initialized.
         *
         * Most services do not need to call this method.
         *
         * \param success Set to \c true if initialization was
         * successful, \c false otherwise.
         *
         * \sa Service::Service
         */
        void setServiceInitialized( bool success );

    private:
        class Private;
        Private* const d;
    };
}

/**
 * Export a %Nepomuk service.
 *
 * \param classname The name of the Nepomuk::Service subclass to export.
 * \param libname The name of the library which should export the service.
 */
#define NEPOMUK_EXPORT_SERVICE( classname, libname )    \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory(#libname))

#endif

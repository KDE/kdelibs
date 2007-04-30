/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _NEPOMUK_REGISTRY_DBUS_BACKEND_H_
#define _NEPOMUK_REGISTRY_DBUS_BACKEND_H_

#include <QtCore/QObject>

#include <knepomuk/servicedesc.h>

namespace Nepomuk {
    namespace Backbone {

	class ServicePublisher;

	namespace DBus {

	    class RegistryInterface;

	    /**
	     * DBus Registry backend.
	     *
	     * \internal This class is not part of the public API
	     */
	    class RegistryBackend : public QObject
		{
		    Q_OBJECT

		public:
		    RegistryBackend( QObject* parent = 0 );
		    ~RegistryBackend();

		    int registerService( ServicePublisher* );
		    int unregisterService( ServicePublisher* );

		    ServiceDesc discoverServiceByUrl( const QString& url );
		    ServiceDesc discoverServiceByType( const QString& type );
		    QList<ServiceDesc> discoverServicesByName( const QString& name );
		    QList<ServiceDesc> discoverServicesByType( const QString& type );
		    QList<ServiceDesc> discoverAllServices();

		Q_SIGNALS:
		    void serviceRegistered( const ServiceDesc& url );
		    void serviceUnregistered( const ServiceDesc& url );

		private:
		    RegistryInterface* m_interface;
		};
	}
    }
}

#endif

/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef _NEPOMUK_REGISTRY_DBUS_INTERFACE_H_
#define _NEPOMUK_REGISTRY_DBUS_INTERFACE_H_

#include <QtDBus>

#include <knepomuk/servicedesc.h>

namespace Nepomuk {
    namespace Backbone {
	namespace DBus {
	    /**
	     * @Private proxy interface for the registry
	     */
	    class RegistryInterface : public QDBusInterface
		{
		    Q_OBJECT

		public:
		    RegistryInterface( QObject* parent = 0 );
		    ~RegistryInterface();

		public Q_SLOTS:
		    int registerService( const ServiceDesc& desc );
		    int unregisterService( const ServiceDesc& desc );
		    int unregisterService( const QString& url );

		    ServiceDesc discoverServiceByUrl( const QString& url );
		    ServiceDesc discoverServiceByType( const QString& type );
		    QList<ServiceDesc> discoverServicesByName( const QString& name );
		    QList<ServiceDesc> discoverServicesByType( const QString& type );
		    QList<ServiceDesc> discoverAllServices();

		Q_SIGNALS:
		    void serviceRegistered( const ServiceDesc& url );
		    void serviceUnregistered( const ServiceDesc& url );
		};
	}
    }
}

#endif

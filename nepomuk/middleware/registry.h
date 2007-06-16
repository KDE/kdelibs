/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _NEPOMUK_REGISTRY_INTERFACE_H_
#define _NEPOMUK_REGISTRY_INTERFACE_H_

#include "nepomuk_export.h"

#include <QtCore/QObject>

namespace Nepomuk {
    namespace Middleware {

	class ServicePublisher;
	class Service;
	class ServiceDesc;

	/**
	 * \brief The Registry is the local abstraction of the \a Nepomuk-KDE service registry. It provides
	 * convenience methods for the functionality of the registry.
	 *
	 * The Registry works with an internal service cache which means that it will keep instances
	 * of all services (as Service objects) in the memory at all times. Thus, the service objects
	 * as returned by the discover methods are not to be deleted by the user but are maintained 
	 * by the Registry instance itself.
	 *
	 * \sa \ref knepclient
	 *
	 * FIXME: introduce some status methos which checks if the registry is actually available.
	 */
	class NEPOMUK_MIDDLEWARE_EXPORT Registry : public QObject
	    {
		Q_OBJECT
	    
	    public:
		/**
		 * Constructs a new Registry objects, connects to the Nepomuk registry and updates
		 * the internal service cache.
		 */
		Registry( QObject* parent = 0 );
		~Registry();
	
		/**
		 * Register a new service with the service registry.
		 *
		 * \return 0 on success, an error code otherwise (The error codes have yet to be defined.)
		 *
		 * \see ServicePublisher
		 */  
		int registerService( ServicePublisher* );

		/**
		 * Unregister a service from the service registry.
		 *
		 * \return 0 on success, an error code otherwise (The error codes have yet to be defined.)
		 *
		 * \see ServicePublisher
		 */  
		int unregisterService( ServicePublisher* );

		/**
		 * Convenience method. Does the same as the above.
		 *
		 * \return 0 on success, an error code otherwise (The error codes have yet to be defined.)
		 */
		int unregisterService( const QString& url );
	  
		Service* discoverServiceByUrl( const QString& url );
		Service* discoverServiceByType( const QString& type );
		QList<Service*> discoverServicesByType( const QString& type );
		QList<Service*> discoverServicesByName( const QString& type );
		QList<Service*> discoverAllServices();

		/**
		 * Convenience method which wraps discoverServicesByType to discover
		 * a core service.
		 */
		Service* discoverRDFRepository();
	  
	    Q_SIGNALS:
		void serviceRegistered( const QString& );
		void serviceUnregistered( const QString& );
		void serviceRegistered( Service* );
		void serviceUnregistered( Service* );

	    private Q_SLOTS:
		void slotServiceRegistered( const ServiceDesc& );
		void slotServiceUnregistered( const ServiceDesc& );
	  
	    private:
		void updateServiceCache();

		class Private;
		Private* const d;
	    };
    }
}

#endif


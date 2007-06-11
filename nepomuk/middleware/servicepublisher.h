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

#ifndef _NEPOMUK_SERVICE_PUBLISHER_H_
#define _NEPOMUK_SERVICE_PUBLISHER_H_

#include "nepomuk_export.h"

#include <QtCore/QObject>
#include <QtCore/QString>


namespace Nepomuk {
    namespace Middleware {

	class ServiceDesc;

	/**
	 * \brief The ServicePublisher is the base class for all services to be registered with the Registry.
	 *
	 * Publishing a service consists of three steps:
	 * \li Create the interface for the service type. This consists of an abstract subclass of
	 *     ServicePublisher which defines the interface for services of the new type and a subclass
	 *     of DBus::ServicePublisherInterface which defines the same interface for the DBus world.
	 *     (Most common Nepomuk services are already part of the knep library. For new services 
	 *     please use the tools FIXME.)
	 * \li Derive from the typed ServicePublisher and implement the abstract methods.
	 * \li Register the service implented in step 2 via Registry::registerService
	 *
	 * \b Example:
	 * \code
	 * class MyQueryService : public QueryServicePublisher
	 * {
	 *    Q_OBJECT
	 *
	 *    public:
	 *     MyQueryService( QObject* parent );
	 *
	 *    public Q_SLOTS:
	 *     QString executeQuery( const QString defaultGraphId,
	 *	                         const QStringList& graphIds,
	 *                           const QString& query );
	 *
	 *     QString executeQuery( const QString& defaultGraphId, 
	 *                           const QString& query );
	 *
	 *     QString executeQuery( const QString& query );
	 * };
	 * \endcode
	 *
	 * Now simply implement the executeQuery methods to perform the queries on your storage and
	 * register the new service via Registry::registerService.
	 *
	 * The Registry will take care of registering the service with the \a %Nepomuk registry and 
	 * publishing the service interface via DBus.
	 *
	 * \sa \ref knepservice
	 */
	class NEPOMUK_EXPORT ServicePublisher : public QObject
	    {
		Q_OBJECT

	    public:
		virtual ~ServicePublisher();

		/**
		 * The unique name of the service
		 */
		QString name() const;

		/**
		 * The URL of the service.
		 *
		 * The service URL is used to locale the service in the system.
		 * It depends on the actual used communication backend.
		 */
		QString url() const;

		/**
		 * Location of the WSDL file describing the interface exposed by
		 * this service.
		 */
		QString type() const;

		ServiceDesc desc() const;

		/**
		 * Set an error code in case a method fails. This will be called automatically
		 * before each method call to reset the error code to success. Thus, there is no
		 * need to use setError in case of success.
		 */
		void setError( const QString& errorName, const QString& msg = QString() );

		/**
		 * The error of the last method call. To be by the DBus communication interface.
		 */
		QString errorName() const;

		/**
		 * The error message of the last method call. To be by the DBus communication interface.
		 */
		QString errorMessage() const;

		/**
		 * Equals to
		 * \code
		 * errorCode() == Error::NoError
		 * \endcode
		 */
		bool success() const;

	    protected:
		explicit ServicePublisher( const QString& name, const QString& url, const QString& type );

	    private:
		class Private;
		Private* const d;
	    };
    }
}


#include "registry.h"

/**
 * \relates Nepomuk::Middleware::ServicePublisher
 * The NEPOMUK_KDE_SERVICE_MAIN macro can be used to create a very simple 
 * application around a Nepomuk::Middleware::ServicePublisher implementation.
 *
 * \b Usage:
 * \code
 * class MyServiceImplementation : public SomeServicePublisher
 * {
 * [...]
 * };
 *
 * NEPOMUK_KDE_SERVICE_MAIN(MyServiceImplementation)
 * \endcode
 */
#define NEPOMUK_KDE_SERVICE_MAIN(ServiceType) \
int main( int argc, char **argv ) \
{ \
  QCoreApplication app( argc, argv ); \
  Nepomuk::Middleware::Registry r; \
  if( !r.registerService( new ServiceType() ) ) \
    return app.exec(); \
  else \
    return 1; \
}

#endif

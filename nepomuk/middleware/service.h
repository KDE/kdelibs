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

#ifndef _NEPOMUK_SERVICE_H_
#define _NEPOMUK_SERVICE_H_

#include "nepomuk_export.h"

#include <QtCore/QObject>

namespace Nepomuk {
    namespace Middleware {

	class Message;
	class Result;
	class ServiceDesc;

	/**
	 * \brief A Service object is the local representation of a \a %Nepomuk service.
	 * Service objects can only be created by the Registry and accessed via its
	 * discover methods.
	 *
	 * The most convenience way to use a Service is to create and instance of a 
	 * ServiceWrapper class providing the interface you need. If no such class
	 * exists (which could be the case for a user defined service type) Service
	 * provides the methodCall method which can be used to call arbitrary methods.
	 *
	 * \sa ServiceWrapper, Registry, \ref knepclient
	 */
	class NEPOMUK_MIDDLEWARE_EXPORT Service : public QObject
	    {
		Q_OBJECT

	    public:
		~Service();

		const QString& type() const;

		/**
		 * Check if the service is still available, i.e. the component
		 * providing the service is still running.
		 *
		 * \return true if the service is available and can be used.
		 */
		bool isAvailable() const;

		/**
		 * \return The unique Nepomuk service URL
		 */
		const QString& url() const;

		const QString& name() const;

		const ServiceDesc& desc() const;

		Result methodCall( const Message& ) const;

	    private:
		explicit Service( const ServiceDesc& desc, QObject* parent = 0 );

		class Private;
		Private* const d;

		friend class Registry;
	    };
    }
}

#endif

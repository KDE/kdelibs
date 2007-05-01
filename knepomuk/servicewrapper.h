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


#ifndef _NEPMUK_BACKBONE_SERVICE_WRAPPER_H_
#define _NEPMUK_BACKBONE_SERVICE_WRAPPER_H_

#include <knepomuk/knepomuk_export.h>

#include <QtCore/QString>

namespace Nepomuk {
    namespace Backbone {

	class Service;
	class Result;

	/**
	 * \brief The ServiceWrapper can be used as a basis for service wrappers which provide
	 * convenience methods wrapping Service::methodCall.
	 *
	 * In most cases a service wrapper class will provide an exact copy of the
	 * interface of its ServicePublisher counterpart.
	 *
	 * In the future ServiceWrapper subclasses should not be created manually but 
	 * by the KNep toolchain.
	 *
	 * Example:
	 * \code
	 * Service* s = registry->discoverServiceByType( "storage.rdf.Query" );
	 * QueryServiceWrapper qs( s );
	 * qs.executeQuery( ... );
	 * \endcode
	 */
	class KNEPOMUK_EXPORT ServiceWrapper
	    {
	    public:
		ServiceWrapper( Service* );
		virtual ~ServiceWrapper();

		Service* service() const;

		int lastStatus() const;
		QString lastErrorName() const;
		QString lastErrorMessage() const;

		/**
		 * This is the same as calling status() == 0.
		 *
		 * \return true if the last method call was successful. Otherwise false.
		 *
		 * \sa lastStatus, lastErrorName, and lastErrorMessage
		 */
		bool success() const;

	    protected:
		/**
		 * Used by subclasses to set the result of the last method call.
		 */
		void setLastResult( const Result& );

	    private:
		class Private;
		Private* const d;
	    };
    }
}


#endif

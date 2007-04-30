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


#ifndef _NEPMUK_BACKBONE_RESOURCEID_SERVICE_H_
#define _NEPMUK_BACKBONE_RESOURCEID_SERVICE_H_

#include <knepomuk/service.h>
#include <knepomuk/servicewrapper.h>
#include <knepomuk/knepomuk_export.h>

//krazy:excludeall=dpointer
namespace Nepomuk {
    namespace Services {

	/**
	 * \brief Convenience wrapper class for an easy usage of
	 * a service of type
	 * %http://nepomuk.semanticdesktop.org/services/resource/identification
	 *
	 * \b Usage:
	 *
	 * \code
	 * Service* s = registry->discoverServiceByType( "http://nepomuk.semanticdesktop.org/services/resource/identification" );
	 * ResourceIdService ids( s );
	 * QString myUrl = ids.toUniqueUrl( "file://foo/bar" );
	 * \endcode
	 */
	class KNEP_EXPORT ResourceIdService : public Backbone::ServiceWrapper
	    {
	    public:
		ResourceIdService( Backbone::Service* );

	    public:
		/**
		 * \see ResourceIdServicePublisher::toUniqueUrl
		 */
		QString toUniqueUrl( const QString& url );

		/**
		 * \see ResourceIdServicePublisher::fromUniqueUrl
		 */
		QString fromUniqueUrl( const QString& url );
	    };
    }
}

#endif

/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
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

#ifndef _NEPMUK_BACKBONE_RESOURCE_ID_SERVICE_PUBLISHER_H_
#define _NEPMUK_BACKBONE_RESOURCE_ID_SERVICE_PUBLISHER_H_

#include <knepomuk/servicepublisher.h>
#include <knepomuk/knepomuk_export.h>


namespace Nepomuk {
    namespace Backbone {
	namespace Services {
	    /**
	     * \brief The Resource Identification Service converts local URLs to unique Nepomuk
	     * URLs and vice versa.
	     */
	    class KNEPOMUK_EXPORT ResourceIdServicePublisher : public ServicePublisher
		{
		    Q_OBJECT

		public:
		    ResourceIdServicePublisher( const QString& name, const QString& url );
		    ~ResourceIdServicePublisher();

		public Q_SLOTS:
		    /**
		     * Converts a desktop URL (a desktop URL can for example be a filename
		     * or a web address) to a unique Nepomuk URL which uniquely identifies
		     * the resource in the Nepomuk system.
		     *
		     * This method is the exact opposite of fromUniqueUrl
		     */
		    virtual QString toUniqueUrl( const QString& url ) = 0;

		    /**
		     * Converts a unique Nepomuk URL to the desktop URL (i.e. the one globally
		     * usable)
		     *
		     * This method is the exact opposite of toUniqueUrl
		     */
		    virtual QString fromUniqueUrl( const QString& url ) = 0;
		};
	}
    }
}

#endif

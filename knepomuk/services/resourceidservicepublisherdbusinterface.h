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

#ifndef _NEPMUK_BACKBONE_RESOURCE_ID_SERVICE_PUBLISHER_DBUS_INTERFACE_H_
#define _NEPMUK_BACKBONE_RESOURCE_ID_SERVICE_PUBLISHER_DBUS_INTERFACE_H_

#include <knepomuk/servicepublisherdbusinterface.h>
#include <knepomuk/knepomuk_export.h>

namespace Nepomuk {
    namespace Backbone {
	namespace Services {
	    class ResourceIdServicePublisher;

	    namespace DBus {
		class KNEPOMUK_EXPORT ResourceIdServicePublisherInterface : public Backbone::DBus::ServicePublisherInterface
		    {
			Q_OBJECT
			Q_CLASSINFO("D-Bus Interface", "org.semanticdesktop.nepomuk.services.resource.Identification")

		    public:
			ResourceIdServicePublisherInterface( ServicePublisher* );

		    public Q_SLOTS:
			QString toUniqueUrl( const QString& url );
			QString fromUniqueUrl( const QString& url );
		    };
	    }
	}
    }
}

#endif

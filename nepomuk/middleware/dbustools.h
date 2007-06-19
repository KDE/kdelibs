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

#ifndef _NEPOMUK_DBUS_TOOLS_H_
#define _NEPOMUK_DBUS_TOOLS_H_

#include <QtCore/QString>
#include <QtDBus/QDBusArgument>

#include "servicedesc.h"
#include "nepomuk_export.h"


NEPOMUK_MIDDLEWARE_EXPORT QDBusArgument& operator<<( QDBusArgument&, const Nepomuk::Middleware::ServiceDesc& desc );
NEPOMUK_MIDDLEWARE_EXPORT const QDBusArgument& operator>>( const QDBusArgument&, Nepomuk::Middleware::ServiceDesc& desc );

namespace Nepomuk {
    namespace Middleware {
	namespace DBus {
	    /**
	     * Converts a Nepomuk service URI into a DBus service name.
	     */
	    NEPOMUK_MIDDLEWARE_EXPORT QString dbusServiceFromUrl( const QString& url );

	    NEPOMUK_MIDDLEWARE_EXPORT QString dbusObjectFromType( const QString& url );
	    NEPOMUK_MIDDLEWARE_EXPORT QString dbusInterfaceFromType( const QString& url );
	}
    }
}

#endif

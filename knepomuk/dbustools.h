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

#ifndef _NEPOMUK_DBUS_TOOLS_H_
#define _NEPOMUK_DBUS_TOOLS_H_

#include <QtCore/QString>
#include <QtDBus/QDBusArgument>

#include <knepomuk/servicedesc.h>
#include <knepomuk/knepomuk_export.h>


KNEPOMUK_EXPORT QDBusArgument& operator<<( QDBusArgument&, const Nepomuk::Backbone::ServiceDesc& desc );
KNEPOMUK_EXPORT const QDBusArgument& operator>>( const QDBusArgument&, Nepomuk::Backbone::ServiceDesc& desc );

namespace Nepomuk {
    namespace Backbone {
	namespace DBus {
	    /**
	     * Converts a Nepomuk service URI into a DBus service name.
	     */
	    KNEPOMUK_EXPORT QString dbusServiceFromUrl( const QString& url );

	    KNEPOMUK_EXPORT QString dbusObjectFromType( const QString& url );
	    KNEPOMUK_EXPORT QString dbusInterfaceFromType( const QString& url );
	}
    }
}

#endif

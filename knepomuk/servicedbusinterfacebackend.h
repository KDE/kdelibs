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

#ifndef _NEPOMUK_SERVICE_DBUS_INTERFACE_H_
#define _NEPOMUK_SERVICE_DBUS_INTERFACE_H_

#include <QtDBus>

namespace Nepomuk {
    namespace Backbone {

	class Message;
	class Result;

	namespace DBus {
	    /**
	     * @Private proxy interface for the service
	     */
	    class ServiceBackend : public QDBusInterface
		{
		    Q_OBJECT

		public:
		    ServiceBackend( const QString& url, const QString& type, QObject* parent = 0 );
		    ~ServiceBackend();

		    bool isAvailable() const;

		    Result methodCall( const Message& message );
		};
	}
    }
}

#endif
